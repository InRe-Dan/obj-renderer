#include "Camera.h"

#include "vectors.h"
#include "Scene.h"
#include "Light.h"

#include "DrawingWindow.h"
#include "Utils.h"
#include "TextureMap.h"
#include "ModelTriangle.h"
#include "RayTriangleIntersection.h"

#include <glm/glm.hpp>
#include <ranges>
#include <vector>
#include <thread>
#include <fstream>

// Takes initializes a camera looking into -z
Camera::Camera(RenderSettings settings, glm::vec3 position)
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

void Camera::updateSettings(RenderSettings newSettings)
{
	this->settings = newSettings;
	frameBuffer = std::vector<std::vector<glm::vec4>>(settings.resolution.y, std::vector<glm::vec4>(settings.resolution.x));
	depthBuffer = std::vector<std::vector<float>>(settings.resolution.y, std::vector<float>(settings.resolution.x, 0.0f));
}

void Camera::lookAt(glm::vec3* target)
{
	lookTarget = target;
}

void Camera::update()
{
	if (lookTarget != nullptr)
	{
		glm::vec3 forward = glm::normalize(*lookTarget - getPosition());
		glm::vec3 right = glm::normalize(glm::cross(forward, { 0, 1, 0 }));
		glm::vec3 up = -glm::normalize(glm::cross(forward, glm::vec3(right)));
		glm::vec3 pos = getPosition();
		placement = glm::mat4(
			glm::vec4(right, pos.x),
			glm::vec4(up, pos.y),
			glm::vec4(forward, pos.z),
			glm::vec4(0, 0, 0, 1));
	}
}

glm::mat4 Camera::getPlacement() const
{
	return placement;
}

glm::vec3 Camera::getPosition() const
{
	return glm::vec3(placement[0][3], placement[1][3], placement[2][3]);
}

void Camera::setPosition(glm::vec3 pos)
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
	glm::vec3 pos = getPosition();
	placement = glm::mat4(o);
	setPosition(pos);
}

// Project a point onto a CanvasPoint
glm::vec2 Camera::getCanvasIntersectionPoint(glm::vec3 vertexLocation) const
{
	// https://www.scratchapixel.com/lessons/3d-basic-rendering/computing-pixel-coordinates-of-3d-point/mathematics-computing-2d-coordinates-of-3d-points.html
	// All coordinates are relative to the camera!
	glm::vec3 vertexToCamera = (vertexLocation - getPosition()) * getOrientation();
	float u = focalLength * (vertexToCamera.x / vertexToCamera.z) +
		imagePlaneWidth / 2;
	float v = focalLength * (-vertexToCamera.y / vertexToCamera.z) +
		(settings.resolution.y * imagePlaneWidth / settings.resolution.x) / 2;
	return glm::vec2{
		glm::floor(settings.resolution.x * (u / imagePlaneWidth)),
		glm::floor(
			settings.resolution.y * v
			/ (settings.resolution.y * imagePlaneWidth / settings.resolution.y)) };
}

glm::vec3 Camera::getRayDirection(glm::uvec2 pixel) const
{
	glm::mat3 o = getOrientation();
	glm::vec3 right = glm::normalize(o[0]);
	glm::vec3 up = glm::normalize(o[1]);
	glm::vec3 forward = glm::normalize(o[2]);
	float pixelLength = imagePlaneWidth / settings.resolution.x;
	glm::vec3 imagePlaneTopLeft = forward * focalLength +
		(up * (settings.resolution.y * 0.5f * pixelLength)) +
		(-right * (pixelLength * settings.resolution.x * 0.5f));
	return glm::normalize(
		imagePlaneTopLeft + float(pixel.x) * pixelLength * right +
		float(pixel.y) * -up * pixelLength);
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
			glm::vec3 e0n = tri.vertexNormals[1] - tri.vertexNormals[0];
			glm::vec3 e1n = tri.vertexNormals[2] - tri.vertexNormals[0];
			glm::vec3 newN = tri.vertexNormals[0] * hit.solution.y
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
		glm::vec3 normalSample = normalMap.value().sample(textureSampleUv);

		glm::vec3 tangentSpaceNormal{ normalSample.x, normalSample.y, normalSample.z };
		// Find axis and angle of rotation
		float angle = glm::acos(glm::dot(glm::vec3(0, 0, 1), (normal)));
		glm::vec3 axis = glm::cross(glm::vec3(0, 0, 1), normal);
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
	glm::vec3 rayOrigin,
	glm::vec3 ray,
	const Scene& scene) const
{
	glm::vec3 closestSolution = glm::vec3(1e10, 0, 0);

	const ModelTriangle* hit = nullptr;
	for (const ModelTriangle& triangle : scene.getModelTriangles())
	{
		glm::vec3 e0 = glm::vec3(triangle.vertices[1] - triangle.vertices[0]);
		glm::vec3 e1 = glm::vec3(triangle.vertices[2] - triangle.vertices[0]);
		glm::vec3 SPVector = rayOrigin - glm::vec3(triangle.vertices[0]);
		glm::mat3 DEMatrix(-ray, e0, e1);
		glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
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
void Camera::drawRtPixel(const glm::uvec2& p, const Scene& scene)
{
	// Cast rays!
	std::vector<RayTriangleIntersection> intersections;
	glm::vec3 rayOrigin = getPosition();
	glm::vec3 rayDirection = getRayDirection(p);
	depthBuffer[p.y][p.x] = -1e10;
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
		return;
	}

	// For now we are only interested in the latest interesection (TODO model scattered light)
	const RayTriangleIntersection& lightOrigin = intersections.back();
	const glm::vec3 originPos = lightOrigin.getPosition();
	const glm::vec3 normal = getNormalOf(lightOrigin);


	glm::vec4 objectCol = lightOrigin.intersectedTriangle
		->sampleDiffuse({ lightOrigin.solution.y, lightOrigin.solution.z }, settings.texturesEnabled);
	
	glm::vec4 ambient = objectCol;
	glm::vec4 diffuse{};
	glm::vec4 specular{};

	for (const Light& lightSource : scene.getLights())
	{
		if (!lightSource.on)
		{
			continue;
		}
		glm::vec3 pointToLight = originPos - lightSource.pos;
		// TODO support reflected light
		const RayTriangleIntersection lightIntersection = getClosestIntersection(originPos, pointToLight, scene);
		if (lightIntersection.intersectedTriangle == nullptr)
		{
			continue;
		}

		float distance = glm::length(pointToLight);
		float distanceSq = static_cast<float>(glm::pow(distance, 2));
		/// Vector of light reflection at this point
		glm::vec3 reflection = glm::normalize(
			(-pointToLight) -2.0f * normal *
						   (glm::dot(-pointToLight, normal)));
		/// Ratio of how much light is being reflected at the camera
		float dotReflection = glm::dot(-rayDirection, reflection);
		dotReflection = glm::clamp(dotReflection, 0.0f, 1.0f);
		float specStrength = glm::pow(dotReflection, lightOrigin.intersectedTriangle->parent->getMaterial().getSpecularExponent());
		float falloff = lightSource.strength / distanceSq;

		diffuse += objectCol * glm::vec4(lightSource.colour, 1.0) * falloff;
		specular += objectCol * glm::vec4(1.0) * falloff * specStrength;
		
	}

	glm::vec4 result = ambient * ambientWeight + diffuse * diffuseWeight + specular * specularWeight;

	frameBuffer[p.y][p.x] = glm::clamp(result, glm::vec4(0.0), glm::vec4(1.0));
	depthBuffer[p.y][p.x] = 1 / glm::length(1 / intersections.front().solution.x);

	//vec3 col = glm::vec3(c.x, c.y, c.z);
	//// Iterate through every light in the scene
	//for (const Light& lightSource : scene.getLights())
	//{
	//	// If light isn't on, skip
	//	if (!lightSource.on)
	//		continue;

	//	float sum = 1.0f;
	//	// Determine if the light can see this point
	//	glm::vec3 lightToPoint = lightSource.pos - intersection.intersectionPoint;
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
	//			 glm::vec3 & lightOffset : offsets)
	//		{
	//			glm::vec3 newLightPoint =
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
	//	glm::vec3 lightImpact = glm::vec3(0);

	//	// Using shading model from
	//	// https://learnopengl.com/Lighting/Basic-Lighting determine brightness
	//	// based on angle of reflection:
	//	glm::vec3 reflection = glm::normalize(
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
	//	// / glm::vec3(255));
	//	lightImpact += originalColour *
	//				   (diffuse * falloffFactor + specular * falloffFactor) *
	//				   glm::vec3(lightSource.colour.r, lightSource.colour.g, lightSource.colour.b);

	//	lightImpact = vecMin(lightImpact, glm::vec3(0.9f));
	//	ambient += lightImpact * sum;
	//}
	//// Cap light at 1.0
	//ambient = vecMin(ambient, glm::vec3(1.0f));
	//intersection.intersectedTriangle.colour =
	//	Colour(ambient.r, ambient.g, ambient.b);
	//return intersection;
}

const std::vector<std::vector<glm::vec4>>& Camera::render(const Scene& scene)
{
	drawFancyBackground();

	switch (settings.mode)
	{
	case RenderSettings::Mode::Raytracing: raytraceRender(scene);
		break;
	default: assert(false);
	}

	return frameBuffer;
}

// Raytrace on a particular section of the internal frame buffer
// Intended to be passed into threads
void Camera::raytraceSection(glm::uvec2 topLeft, glm::uvec2 bottomRight, const Scene& scene)
{
	for (glm::uint i = topLeft.y; i < bottomRight.y; i++)
	{
		for (glm::uint j = topLeft.x; j < bottomRight.x; j++)
		{
			drawRtPixel(glm::uvec2(j, i), scene);
		}
	}
}

void Camera::raytraceRender(const Scene& scene)
{
	std::vector<std::thread> threads;
	size_t slice_height = settings.resolution.y / settings.threadCount;
	for (int i = 0; i < settings.threadCount - 1; i++)
	{
		threads.emplace_back(
			[this, slice_height, &scene, i]
			{
				raytraceSection(
				{ 0, slice_height * i }
				, { settings.resolution.x, slice_height * (i + 1) }
				, scene);
			});
	}

	threads.emplace_back(
		[this, slice_height, &scene]
		{
			raytraceSection(
				{ 0, slice_height * (settings.threadCount - 1) }
				, { settings.resolution.x, settings.resolution.y }
				, scene);
		});

	for (auto& thread : threads)
	{
		thread.join();
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
	glm::vec3 topLeft = glm::normalize(getRayDirection({ 0, 0 }));
	glm::vec3 topRight = glm::normalize(getRayDirection({ settings.resolution.x, 0 }));
	glm::vec3 bottomLeft = glm::normalize(getRayDirection({ 0, settings.resolution.y }));
	glm::vec3 bottomRight =
		glm::normalize(getRayDirection({ settings.resolution.x, settings.resolution.y }));
	std::vector<glm::vec3> leftEdge = interpolate(topLeft, bottomLeft, settings.resolution.y);
	std::vector<glm::vec3> rightEdge = interpolate(topRight, bottomRight, settings.resolution.y);
	for (int i = 0; i < settings.resolution.y; i++)
	{
		std::vector<glm::vec3> horizontalLine =
			interpolate(leftEdge[i], rightEdge[i], settings.resolution.x);
		for (int j = 0; j < settings.resolution.x; j++)
		{
			glm::vec3 col = horizontalLine[j] * 0.5f + glm::vec3(0.5f);
			frameBuffer[i][j] = { col.x, col.y, col.z, 1.0 };
			depthBuffer[i][j] = -1e10;
		}
	}
}

// Add a std::vector to camera position
void Camera::moveBy(glm::vec3 vect)
{
	placement[0][3] += vect.x;
	placement[1][3] += vect.y;
	placement[2][3] += vect.z;
}

// Move in the "Up" direction relative to view
void Camera::moveUp(float a)
{
	moveBy(a * glm::normalize(glm::vec3(placement[1])));
}

// Move in the negative "Up" direction relative to view
void Camera::moveDown(float a)
{
	moveUp(-a);
}

// Move in the "Right" direction relative to view
void Camera::moveRight(float a)
{
	moveBy(a * glm::normalize(glm::vec3(placement[0])));
}

// Move in the negative "Right" direction relative to view
void Camera::moveLeft(float a)
{
	moveRight(-a);
}

// Move in the "Forward" direction relative to view
void Camera::moveForward(float a)
{
	moveBy(a * glm::normalize(glm::vec3(placement[2])));
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
