#pragma once

#include "vectors.h"
#include "Scene.h"
#include "Light.h"
#include "DrawingWindow.h"
#include "Utils.h"
#include "TextureMap.h"
#include "ModelTriangle.h"
#include "RayTriangleIntersection.h"

#include <glm/glm.hpp>
#include <vector>
#include <thread>
#include <fstream>
#include <optional>


// Object to represent a camera in a scene.
class Camera
{
public:

	static constexpr float ambientWeight = 0.05f;
	static constexpr float specularWeight = 0.3f;
	static constexpr float diffuseWeight = 0.65f;

	/// Camera rendering configuration.
	struct RenderSettings
	{
		enum class Mode
		{
			Raytracing
		};

		enum class Smoothing
		{
			None,
			Gouraud,
			Phong
		};

		Mode mode = Mode::Raytracing;

		size_t threadCount = 10;
		glm::u64vec2 resolution = { 640, 480 };

		bool lightingEnabled = false;
		bool texturesEnabled = false;
		bool normalMapsEnabled = false;
		bool lightPositionPreview = true;
		size_t raycastIterations = 6;
		Smoothing smoothing = Smoothing::None;
	};


	// Takes initializes a camera looking into -z
	Camera(RenderSettings settings, glm::vec3 position = { 0, 0, 10 });

	RenderSettings getSettings() const;
	void updateSettings(RenderSettings settings);

	// Look at the position of this std::vector. Will function if this value changes.
	void lookAt(glm::vec3* target = nullptr);

	// Clears frame buffers and looks towards the target if looking is on.
	void update();

	// Get placement matrix. Right side represents postiion.
	glm::mat4 getPlacement() const;

	// Get position of camera from placement matrix.
	glm::vec3 getPosition() const;

	// Set the position values in placement matrix.
	void setPosition(glm::vec3 pos);

	// Get an orientation matrix from placement
	glm::mat3 getOrientation() const;

	void setOrientation(glm::mat3 o);

	// Project a point onto a CanvasPoint
	glm::vec2 getCanvasIntersectionPoint(glm::vec3 vertexLocation) const;

	// Get ray direction for a given pixel on the internal frame buffer
	glm::vec3 getRayDirection(glm::uvec2 pixel) const;

	// Find closest intersection in the direction of ray from rayOrigin.
	RayTriangleIntersection getClosestIntersection(
		glm::vec3 rayOrigin,
		glm::vec3 ray,
		const Scene& scene
	) const;

	// Return the intersection information for a given pixel on the internal
	// frame buffer.
	void drawRtPixel(const glm::uvec2& pixel, const Scene& scene);

	const std::vector<std::vector<glm::vec4>>& render(const Scene& scene);

	// Draw a backdrop to the frame buffer that visualizes each axis as the
	// presence or absence of a colour channel.
	void drawFancyBackground();

	// Add a std::vector to camera position
	void moveBy(glm::vec3 vect);

	// Move in the "Up" direction relative to view
	void moveUp(float a);

	// Move in the negative "Up" direction relative to view
	void moveDown(float a);

	// Move in the "Right" direction relative to view
	void moveRight(float a);

	// Move in the negative "Right" direction relative to view
	void moveLeft(float a);

	// Move in the "Forward" direction relative to view
	void moveForward(float a);

	// Move in the negative "Forward" direction relative to view
	void moveBack(float a);

	// Tilt view up around x axis
	void lookUp(float degrees);
	// Tilt view down around x axis
	void lookDown(float degrees);

	// Tilt view right around y axis
	void lookRight(float degrees);

	// Tilt view left around y axis
	void lookLeft(float degrees);

	// Add some amount to focal length
	void changeF(float diff);

	// Get unit width of image plane
	float getImagePlaneWidth();

	// Get focal length
	float getFocalLength();

	// Add w, h to resolution. Recreates frame and depth buffers.
	void changeResolutionBy(int w, int h);

	const std::vector<std::vector<float>>& getDepth() const;

private:

	glm::vec3 getNormalOf(const RayTriangleIntersection& intersection) const;

	// Raytrace on a particular section of the internal frame buffer
	// Intended to be passed into threads
	void raytraceSection(glm::uvec2 topLeft, glm::uvec2 bottomRight, const Scene& scene);

	// Raytraced render on the internal frame buffer. Uses threadCount threads.
	void raytraceRender(const Scene& scene);


	RenderSettings settings{};
	// Internal frame buffer
	std::vector<std::vector<glm::vec4>> frameBuffer;
	// Internal depth buffer
	std::vector<std::vector<float>> depthBuffer;
	// Address of the position of the target
	glm::vec3* lookTarget = nullptr;
	// Distance to image plane. Should be positive!
	float focalLength;
	// Unit length of image place. Height to be derived from aspect ratio.
	float imagePlaneWidth;
	// Placement matrix representing orientation and position
	glm::mat4 placement;
};