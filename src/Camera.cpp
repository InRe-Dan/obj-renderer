#include "Camera.h"

#include "utility/vectors.h"
#include "utility/drawing.h"
#include "Scene.h"
#include "Light.h"

#include "CanvasPoint.h"
#include "CanvasTriangle.h"
#include "Colour.h"
#include "DrawingWindow.h"
#include "Utils.h"
#include "TextureMap.h"
#include "ModelTriangle.h"
#include "RayTriangleIntersection.h"

#include <GLM/glm.hpp>
#include <ranges>
#include <vector>
#include <thread>
#include <fstream>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using std::vector;

// Takes initializes a camera looking into -z
Camera::Camera(RenderSettings settings, vec3 position)
{
	imagePlaneWidth = 5.0f;
	focalLength = 3;
	placement = glm::mat4(
		1,
		0,
		0,
		position.x,
		0,
		1,
		0,
		position.y,
		0,
		0,
		-1,
		position.z,
		0,
		0,
		0,
		0);
	updateSettings(settings);
}

Camera::RenderSettings Camera::getSettings() const
{
	return settings;
}

void Camera::updateSettings(RenderSettings settings)
{
	this->settings = settings;
	frameBuffer = std::vector<std::vector<uint32_t>>(settings.resolution.y, std::vector<uint32_t>(settings.resolution.x, 0));
	depthBuffer = std::vector<std::vector<float>>(settings.resolution.y, std::vector<float>(settings.resolution.x, 0.0f));
}

void Camera::lookAt(vec3* target)
{
	lookTarget = target;
}

void Camera::update()
{
	if (lookTarget != nullptr)
	{
		vec3 forward = glm::normalize(*lookTarget - getPosition());
		vec3 right = glm::normalize(glm::cross(vec3(forward), vec3(0, 1, 0)));
		vec3 up = -glm::normalize(glm::cross(vec3(forward), vec3(right)));
		vec3 pos = getPosition();
		placement = glm::mat4(
			vec4(right, pos.x),
			vec4(up, pos.y),
			vec4(forward, pos.z),
			vec4(0, 0, 0, 1));
	}
}

glm::mat4 Camera::getPlacement() const
{
	return placement;
}

vec3 Camera::getPosition() const
{
	return vec3(placement[0][3], placement[1][3], placement[2][3]);
}

void Camera::setPosition(vec3 pos)
{
	placement[0][3] = pos.x;
	placement[1][3] = pos.y;
	placement[2][3] = pos.z;
}

glm::mat3 Camera::getOrientation() const
{
	return glm::mat3(placement);
}

void Camera::setOrientation(glm::mat3 o)
{
	vec3 pos = getPosition();
	placement = glm::mat4(o);
	setPosition(pos);
}

// Project a point onto a CanvasPoint
CanvasPoint Camera::getCanvasIntersectionPoint(vec3 vertexLocation) const
{
	// https://www.scratchapixel.com/lessons/3d-basic-rendering/computing-pixel-coordinates-of-3d-point/mathematics-computing-2d-coordinates-of-3d-points.html
	// All coordinates are relative to the camera!
	vec3 vertexToCamera = (vertexLocation - getPosition()) * getOrientation();
	float u = focalLength * (vertexToCamera.x / vertexToCamera.z) +
			  imagePlaneWidth / 2;
	float v = focalLength * (-vertexToCamera.y / vertexToCamera.z) +
			  (settings.resolution.y * imagePlaneWidth / settings.resolution.x) / 2;
	return CanvasPoint(
		glm::floor(settings.resolution.x * (u / imagePlaneWidth)),
		glm::floor(
			settings.resolution.y * v 
			/ (settings.resolution.y * imagePlaneWidth / settings.resolution.y)),
		1 / vertexToCamera.z);
}

vec3 Camera::getRayDirection(int x, int y) const
{
	glm::mat3 o = getOrientation();
	vec3 right = glm::normalize(o[0]);
	vec3 up = glm::normalize(o[1]);
	vec3 forward = glm::normalize(o[2]);
	float pixelLength = imagePlaneWidth / settings.resolution.x;
	vec3 imagePlaneTopLeft = forward * focalLength +
							 (up * (settings.resolution.y * 0.5f * pixelLength)) +
							 (-right * (pixelLength * settings.resolution.x * 0.5f));
	return glm::normalize(
		imagePlaneTopLeft + float(x) * pixelLength * right +
		float(y) * -up * pixelLength);
}

glm::vec3 Camera::getNormalOf(const RayTriangleIntersection& hit) const
{
	assert(hit.intersectedTriangle);
	
	const ModelTriangle& tri = *hit.intersectedTriangle;

	glm::vec2 textureSampleUv =
		tri.triangleToTexture(glm::vec2(hit.solution.y, hit.solution.z));

	glm::vec3 normal = tri.getNormal();

	if (tri.smoothingGroup != 0
		&& settings.smoothing != RenderSettings::Smoothing::None)
	{
		if (settings.smoothing == RenderSettings::Smoothing::Gouraud)
		{
			// Unimplemented
		}
		else
		{
			vec3 e0n = tri.vertexNormals[1] - tri.vertexNormals[0];
			vec3 e1n = tri.vertexNormals[2] - tri.vertexNormals[0];
			vec3 newN = tri.vertexNormals[0] * hit.solution.y
				+ e1n * hit.solution.z;
			normal = newN;
		}
	}

	const std::optional<Surface>& normalMap = hit.intersectedTriangle->parent->getMaterial().getNormalTexture();

	// Apply normal map if there is one available
	if (normalMap && settings.normalMapsEnabled)
	{
		// Generating a matrix to rotate from tangent space to world space
		// Matrix represents rotation from "blue" in tangent space to the base
		// triangle normal https://en.wikipedia.org/wiki/Rotation_matrix
		Colour normalSample = normalMap.value().sample(textureSampleUv);

		vec3 tangentSpaceNormal{ normalSample.r(), normalSample.g(), normalSample.b()};
		// Find axis and angle of rotation
		float angle = glm::acos(glm::dot(vec3(0, 0, 1), (normal)));
		vec3 axis = glm::cross(vec3(0, 0, 1), normal);
		// Precompute frequently needed values
		float c = cos(angle);
		float s = sin(angle);
		float t = 1 - c;
		// Create matrix
		glm::mat3 rotation = glm::mat3(
			c + axis.x * axis.x * t,
			axis.x * axis.y * t - axis.z * s,
			axis.x * axis.z * t + axis.y * s,
			axis.y * axis.x * t + axis.z * s,
			c + axis.y * axis.y * t,
			axis.y * axis.z * t - axis.x * s,
			axis.z * axis.x * t - axis.y * s,
			axis.z * axis.y * t + axis.x * s,
			c + axis.z * axis.z * t);
		// Apply to normalmap normal
		normal = tangentSpaceNormal * rotation;
	}

	return normal;
}

// Find closest intersection in the direction of ray from rayOrigin.
RayTriangleIntersection Camera::getClosestIntersection(
	vec3 rayOrigin,
	vec3 ray,
	const Scene& scene) const
{
	vec3 closestSolution = vec3(1e10, 0, 0);
	
	const ModelTriangle* hit = nullptr;
	for (const ModelTriangle& triangle : scene.getModelTriangles())
	{
		vec3 e0 = vec3(triangle.vertices[1] - triangle.vertices[0]);
		vec3 e1 = vec3(triangle.vertices[2] - triangle.vertices[0]);
		vec3 SPVector = rayOrigin - vec3(triangle.vertices[0]);
		glm::mat3 DEMatrix(-ray, e0, e1);
		vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
		if (possibleSolution.x < closestSolution.x // Is this better than the old?
			&& 0.0f <= possibleSolution.y // Is it within the triangle's first edge?
			&& possibleSolution.y <= 1.0f // ^ 
			&& 0.0f <= possibleSolution.z // Second edge?
			&& possibleSolution.z <= 1.0f // ^
			&& possibleSolution.y + possibleSolution.z <= 1.0f // No further than third edge?
			&& possibleSolution.x > 0.001f) // Mitigate shadow acne
		{
			closestSolution = possibleSolution;
			hit = &triangle;
		}
	}

	return RayTriangleIntersection
	{
		.intersectedTriangle = hit,
		.solution = closestSolution
	};
}

// Return the the colour of a pixel according to raytracing settings.
Colour Camera::getRtPixelColour(int xPos, int yPos, const Scene& scene)
{
	// Cast rays!
	std::vector<RayTriangleIntersection> intersections;
	vec3 rayOrigin = getPosition();
	vec3 rayDirection = getRayDirection(xPos, yPos); 
	for (size_t i = 0; i < settings.raycastIterations; i++)
	{
		intersections.push_back(getClosestIntersection(rayOrigin, rayDirection, scene));
		RayTriangleIntersection& intersection = intersections.back();
		if (intersection.intersectedTriangle == nullptr)
		{
			intersections.pop_back();
			break;
		}
		if (intersection.intersectedTriangle->parent->getMaterial().getReflectivity() == 0.0)
		{
			break;
		}

		rayOrigin += rayDirection * intersection.solution.x;
		rayDirection = getNormalOf(intersection);
	}
	
	if (intersections.empty())
	{
		return Colour{ 0.0, 0.0, 0.0, 0.0 };
	}

	// Everything must be at least 10% brightness

	RayTriangleIntersection lightOrigin = intersections.back();
	intersections.pop_back();

	Colour total_colour = lightOrigin.intersectedTriangle
		->sampleDiffuse({lightOrigin.solution.y, lightOrigin.solution.z}, settings.texturesEnabled);

	return total_colour;

	//vec3 col = vec3(c.x, c.y, c.z);
	//// Iterate through every light in the scene
	//for (const Light& lightSource : scene.getLights())
	//{
	//	// If light isn't on, skip
	//	if (!lightSource.on)
	//		continue;

	//	float sum = 1.0f;
	//	// Determine if the light can see this point
	//	vec3 lightToPoint = lightSource.pos - intersection.intersectionPoint;
	//	if (glm::length(lightToPoint) > 100)
	//		continue;
	//	if (lightSource.radius == 0.0)
	//	{
	//		RayTriangleIntersection lightIntersection = getClosestIntersection(
	//			lightSource.pos,
	//			-lightToPoint,
	//			scene,
	//			0);
	//		if (lightIntersection.triangleIndex != -1 &&
	//			(intersection.triangleIndex != lightIntersection.triangleIndex))
	//		{
	//			// if it can't, skip
	//			continue;
	//		}
	//	}
	//	else
	//	{
	//		// shoot 6 additional rays around the light to determine brightness
	//		for (std::vector<vec3> offsets = generateLightOffsets();
	//			 vec3 & lightOffset : offsets)
	//		{
	//			vec3 newLightPoint =
	//				lightSource.pos + lightOffset * lightSource.radius;
	//			RayTriangleIntersection softIntersection =
	//				getClosestIntersection(
	//					newLightPoint,
	//					-(newLightPoint - intersection.intersectionPoint),
	//					scene,
	//					0);
	//			if (softIntersection.triangleIndex != -1 &&
	//				(intersection.triangleIndex !=
	//				 softIntersection.triangleIndex))
	//				sum -= 1.0 / offsets.size();
	//		}
	//		if (sum < 0.001)
	//			continue;
	//	}

	//	// How much this light will affect brighness
	//	vec3 lightImpact = vec3(0);

	//	// Using shading model from
	//	// https://learnopengl.com/Lighting/Basic-Lighting determine brightness
	//	// based on angle of reflection:
	//	vec3 reflection = glm::normalize(
	//		lightToPoint - 2.0f * intersection.normal *
	//						   (glm::dot(lightToPoint, intersection.normal)));
	//	float dotReflection = glm::dot(-rayDirection, reflection);
	//	if (dotReflection < 0.0f)
	//		dotReflection = 0.0f;
	//	if (dotReflection > 1.0f)
	//		dotReflection = 1.0f;
	//	float specular = glm::pow(
	//						 dotReflection,
	//						 intersection.intersectedTriangle.material
	//							 ->getSpecularExponent()) *
	//					 0.3f;

	//	// determine brighness based on angle of incidence
	//	float dotNormal = glm::dot(lightToPoint, intersection.normal);
	//	if (dotNormal < 0.0f)
	//		dotNormal = 0.0f;
	//	if (dotNormal > 1.0f)
	//		dotNormal = 1.0f;
	//	float diffuse = dotNormal * 0.65f;

	//	// Falloff based on distance from light
	//	float falloffFactor = lightSource.strength / (glm::length(lightToPoint) *
	//											  glm::length(lightToPoint));

	//	// Add to ambient light
	//	// lightImpact += originalColour * quantize(diffuse * falloffFactor +
	//	// specular, 2) * (vec3(lightSource->r, lightSource->g, lightSource->b)
	//	// / vec3(255));
	//	lightImpact += originalColour *
	//				   (diffuse * falloffFactor + specular * falloffFactor) *
	//				   vec3(lightSource.colour.r, lightSource.colour.g, lightSource.colour.b);

	//	lightImpact = vecMin(lightImpact, vec3(0.9f));
	//	ambient += lightImpact * sum;
	//}
	//// Cap light at 1.0
	//ambient = vecMin(ambient, vec3(1.0f));
	//intersection.intersectedTriangle.colour =
	//	Colour(ambient.r, ambient.g, ambient.b);
	//return intersection;
}

const std::vector<std::vector<uint32_t>>& Camera::render(const Scene& scene)
{
	drawFancyBackground();
	
	switch (settings.mode)
	{
	case RenderSettings::Mode::Wireframe: wireframeRender(scene);
		break;
	case RenderSettings::Mode::Raster: rasterRender(scene);
		break;
	case RenderSettings::Mode::Raytracing: raytraceRender(scene);
		break;
	default: assert(false);
	}

	drawLights(scene);

	return frameBuffer;
}

// Raytrace on a particular section of the internal frame buffer
// Intended to be passed into threads
void Camera::raytraceSection(int x1, int x2, int y1, int y2, const Scene& scene)
{
	for (int i = y1; i < y2; i++)
	{
		for (int j = x1; j < x2; j++)
		{
			Colour col = getRtPixelColour(j, i, scene);
			// TODO depthBuffer[i][j] =
				// 1 / glm::length(getPosition() - intersection.intersectionPoint);
			frameBuffer[i][j] = col.pack();
		}
	}
}

// Raster render on the internal frame buffer
void Camera::rasterRender(const Scene& scene)
{
	for (const ModelTriangle& triangle : scene.getModelTriangles())
	{
		CanvasPoint a =
			getCanvasIntersectionPoint(glm::vec3(triangle.vertices[0]));
		CanvasPoint b =
			getCanvasIntersectionPoint(glm::vec3(triangle.vertices[1]));
		CanvasPoint c =
			getCanvasIntersectionPoint(glm::vec3(triangle.vertices[2]));
		// Very unsophisticated clipping - remove if any vertex is outside view
		if (isInBounds(a, vec4(0, 0, settings.resolution.x, settings.resolution.y)) &&
			isInBounds(b, vec4(0, 0, settings.resolution.x, settings.resolution.y)) &&
			isInBounds(c, vec4(0, 0, settings.resolution.x, settings.resolution.y)))
		{
			CanvasTriangle canvasTriangle(a, b, c);
			filledTriangle(
				canvasTriangle,
				triangle.parent->getMaterial().getDiffuseColour().value_or(Colour{1.0, 1.0, 1.0, 1.0}),
				frameBuffer,
				depthBuffer);
		}
	}
}

// Wireframe render on the internal frame buffer
void Camera::wireframeRender(const Scene& scene)
{
	for (const ModelTriangle& triangle : scene.getModelTriangles())
	{
		CanvasPoint a =
			getCanvasIntersectionPoint(glm::vec3(triangle.vertices[0]));
		CanvasPoint b =
			getCanvasIntersectionPoint(glm::vec3(triangle.vertices[1]));
		CanvasPoint c =
			getCanvasIntersectionPoint(glm::vec3(triangle.vertices[2]));
		if (isInBounds(a, vec4(0, 0, settings.resolution.x, settings.resolution.y)) &&
			isInBounds(b, vec4(0, 0, settings.resolution.x, settings.resolution.y)) &&
			isInBounds(c, vec4(0, 0, settings.resolution.x, settings.resolution.y)))
		{
			CanvasTriangle canvasTriangle(a, b, c);
			strokedTriangle(
				canvasTriangle,
				triangle.parent->getMaterial().getDiffuseColour().value_or(Colour{ 1.0, 1.0, 1.0, 1.0 }),
				frameBuffer,
				depthBuffer);
		}
	}
}

void Camera::raytraceRender(const Scene& scene)
{
	vector<std::thread> threads;
	int slice_height = settings.resolution.y / settings.threadCount;
	for (int i = 0; i < settings.threadCount - 1; i++)
	{
		threads.emplace_back(
			[this, slice_height, &scene, i]
			{
				raytraceSection(
					0
					, settings.resolution.x
					, slice_height * i
					, slice_height * (i + 1)
					, scene);
			});
	}

	threads.emplace_back(
		[this, slice_height, &scene]
		{
			raytraceSection(
				0
				, settings.resolution.x
				, slice_height * (settings.threadCount - 1)
				, settings.resolution.y
				, scene);
		});

	for (auto& thread : threads)
	{
		thread.join();
	}
}

void Camera::drawLights(const Scene& scene)
{
	// If previews are disabled, pass
	if (!settings.lightPositionPreview)
		return;
	for (const Light& light : scene.getLights())
	{
		// Don't render lights that are too close
		if ((0.5 < glm::length(light.pos - getPosition()) && light.on))
		{
			// Project to the focal plane
			CanvasPoint lightXY = getCanvasIntersectionPoint(light.pos);
			if ((0 <= lightXY.x) 
				&& (lightXY.x < settings.resolution.x)
				&& (0 <= lightXY.y)
				&& (lightXY.y< settings.resolution.y))
			{
				// Hack an illusion of depth by making circle size inversely
				// proportional to depth
				circle(
					lightXY.x,
					lightXY.y,
					0.1 * lightXY.depth,
					lightXY.depth,
					light.colour,
					frameBuffer,
					depthBuffer);
			}
		}
	}
}

// Draw a backdrop to the frame buffer that visualizes each axis as the presence
// or absence of a colour channel.
void Camera::drawFancyBackground()
{
	/*
	Inspired by a Sebastian Lague video, I think.
	Very much the same idea as one of the opening interpolation tasks. Just
	casts rays to each of the corners of the image plane to determine colours.
	*/
	vec3 topLeft = glm::normalize(getRayDirection(0, 0));
	vec3 topRight = glm::normalize(getRayDirection(settings.resolution.x, 0));
	vec3 bottomLeft = glm::normalize(getRayDirection(0, settings.resolution.y));
	vec3 bottomRight =
		glm::normalize(getRayDirection(settings.resolution.x, settings.resolution.y));
	vector<vec3> leftEdge = interpolate(topLeft, bottomLeft, settings.resolution.y);
	vector<vec3> rightEdge = interpolate(topRight, bottomRight, settings.resolution.y);
	for (int i = 0; i < settings.resolution.y; i++)
	{
		vector<vec3> horizontalLine =
			interpolate(leftEdge[i], rightEdge[i], settings.resolution.x);
		for (int j = 0; j < settings.resolution.x; j++)
		{
			vec3 col = horizontalLine[j] * 0.5f + vec3(0.5f);
			frameBuffer[i][j] = Colour{col.x, col.y, col.z, 1.0}.pack();
		}
	}
}

// Add a vector to camera position
void Camera::moveBy(vec3 vect)
{
	placement[0][3] += vect.x;
	placement[1][3] += vect.y;
	placement[2][3] += vect.z;
}

// Move in the "Up" direction relative to view
void Camera::moveUp(float a)
{
	moveBy(a * glm::normalize(vec3(placement[1])));
}

// Move in the negative "Up" direction relative to view
void Camera::moveDown(float a)
{
	moveUp(-a);
}

// Move in the "Right" direction relative to view
void Camera::moveRight(float a)
{
	moveBy(a * glm::normalize(vec3(placement[0])));
}

// Move in the negative "Right" direction relative to view
void Camera::moveLeft(float a)
{
	moveRight(-a);
}

// Move in the "Forward" direction relative to view
void Camera::moveForward(float a)
{
	moveBy(a * glm::normalize(vec3(placement[2])));
}

// Move in the negative "Forward" direction relative to view
void Camera::moveBack(float a)
{
	moveForward(-a);
}

// Tilt view up around x axis
void Camera::lookUp(float degrees)
{
	placement = getXRotationMatrix(-degrees) * placement;
}

// Tilt view down around x axis
void Camera::lookDown(float degrees)
{
	placement = getXRotationMatrix(degrees) * placement;
}

// Tilt view right around y axis
void Camera::lookRight(float degrees)
{
	placement = getYRotationMatrix(degrees) * placement;
}

// Tilt view left around y axis
void Camera::lookLeft(float degrees)
{
	placement = getYRotationMatrix(-degrees) * placement;
}

// Add some amount to focal length
void Camera::changeF(float diff)
{
	focalLength += diff;
}

// Get unit width of image plane
float Camera::getImagePlaneWidth()
{
	return imagePlaneWidth;
}

// Get focal length
float Camera::getFocalLength()
{
	return focalLength;
}

// Add w, h to resolution. Recreates frame and depth buffers.
void Camera::changeResolutionBy(int w, int h)
{
	settings.resolution.x += w;
	settings.resolution.y += h;
	// TODO add getres and setres, use mdspan for easier resizing

	updateSettings(settings);
}

const std::vector<std::vector<float>>& Camera::getDepth() const
{
	return depthBuffer;
}
