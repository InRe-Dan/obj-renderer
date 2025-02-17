#pragma once

#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <TextureMap.h>
#include <ModelTriangle.h>
#include <functional>
#include <iomanip>

// Easy, simple, fast and reliable rounding for most cases. Saved me from a lot
// of issues.
int roundI(float x);

// Return a string representing this float in a fixed number of characters.
// First character is reserved for a sign. SPACE if float is positive.
std::string formatFloat(float num, int width);

inline float quantize(float value, float level);

// Return the string representing a vector.
std::string printVec(glm::vec3 v);

glm::mat4 getXRotationMatrix(float degrees);

glm::mat4 getYRotationMatrix(float degrees);

glm::mat4 getZRotationMatrix(float degrees);

glm::mat4 getTranslationMatrix(glm::vec3 vect);

std::vector<glm::vec3> generateLightOffsets();

bool isInBounds(CanvasPoint point, glm::vec4 bounds);

// Print a 4x4 matrix to stdout
void output(glm::mat4 matrix, std::string title);

// Clip a vector of canvas points such that all points are within some bounds
std::vector<CanvasPoint>
bindToRectangle(std::vector<CanvasPoint> input, glm::vec2 topLeft, glm::vec2 bottomRight);

// Convert vector to uint32_t (assuming vector values range from 0 and 1)
uint32_t vec3ToColour(glm::vec3 vect, int alpha);

// Convert RGB uint32_t to vector with values ranging from 0 - 255
glm::vec3 intColToVec3(uint32_t c);

std::vector<float> interpolate(float from, float to, int steps);

glm::vec3 vecMin(glm::vec3 one, glm::vec3 two);

std::vector<glm::vec2> interpolate(glm::vec2 from, glm::vec2 to, int steps);

std::vector<glm::vec3> interpolate(glm::vec3 from, glm::vec3 to, int steps);

std::vector<CanvasPoint>
interpolate(CanvasPoint fromC, CanvasPoint toC, int steps);

// Pure virtual class to be implemented by objects that can be controlled by an
// Animation
class Animateable
{
  public:
	virtual glm::vec3 getPosition() = 0;
	virtual void setPosition(glm::vec3 pos) = 0;
};

class Rotateable
{
  public:
	virtual glm::mat3 getOrientation() = 0;
	virtual void setOrientation(glm::mat3 o) = 0;
};

class Animation
{
  public:
	virtual void animate() = 0;
	virtual void toggle() = 0;
};

// Animation controller class.
class Translation : public Animation
{
  public:
	Translation(Animateable* object, std::function<glm::vec3(glm::vec3, int)> posFunc);
	// If animation is enabled, increase step and move the object.
	void animate();
	// Toggle animation between enabled/disabled
	void toggle();

  private:
	Animateable* target;
	glm::vec3 origin;
	std::function<glm::vec3(glm::vec3, int)> f;
	int tick;
	bool on = false;
};

class Rotation : public Animation
{
  public:
	Rotation(Rotateable* object, glm::vec3 degreesPerTick);
	void animate();
	// Toggle animation between enabled/disabled
	void toggle();

  private:
	glm::vec3 dpt;
	Rotateable* target;
	glm::mat3 startOrientation;
	int tick;
	bool on = false;
};

class AdjustableRotation : public Animation
{
  public:
	AdjustableRotation(Rotateable* object, std::function<glm::vec3(int)> degfunc);

	void animate();

	// Toggle animation between enabled/disabled
	void toggle();

  private:
	Rotateable* target;
	glm::mat3 originalOrientation;
	std::function<glm::vec3(int)> f;
	int tick;
	bool on = false;
};