#include "vectors.h"

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

using namespace glm;
using std::string;
using std::vector;

int roundI(float x)
{
	return (x > 0) ? int(x + 0.5) : int(x - 0.5);
}

// Return a string representing this float in a fixed number of characters.
// First character is reserved for a sign. SPACE if float is positive.
string formatFloat(float num, int width)
{
	string returnString;
	string floatString = std::to_string(num);
	bool foundPoint = false;
	int originalWidth = width;
	if (num >= 0.0f)
	{
		width--;
		returnString += " ";
	}
	for (char c : floatString)
	{
		if (c == '.')
		{
			foundPoint = true;
		}
		if (foundPoint && width == 0)
			break;
		returnString += c;
		width--;
	}
	if (width < 0)
	{
		return std::string(">", originalWidth);
	}
	for (int i = 0; i < width; i++)
	{
		returnString += "0";
	}
	return returnString;
}

inline float quantize(float value, float level)
{
	return roundI(value * level) / level;
}

// Return the string representing a vector.
string printVec(vec3 v)
{
	return "(" + formatFloat(v.x, 5) + ", " + formatFloat(v.y, 5) + ", " +
		   formatFloat(v.z, 5) + ")";
}

glm::mat4 getXRotationMatrix(float degrees)
{
	return glm::mat4(
		1,
		0,
		0,
		0,
		0,
		glm::cos(glm::radians(degrees)),
		-glm::sin(glm::radians(degrees)),
		0,
		0,
		glm::sin(glm::radians(degrees)),
		glm::cos(glm::radians(degrees)),
		0,
		0,
		0,
		0,
		1);
}

glm::mat4 getYRotationMatrix(float degrees)
{
	return glm::mat4(
		glm::cos(glm::radians(degrees)),
		0,
		glm::sin(glm::radians(degrees)),
		0,
		0,
		1,
		0,
		0,
		-glm::sin(glm::radians(degrees)),
		0,
		glm::cos(glm::radians(degrees)),
		0,
		0,
		0,
		0,
		1);
}

glm::mat4 getZRotationMatrix(float degrees)
{
	return glm::mat4(
		glm::cos(glm::radians(degrees)),
		-glm::sin(glm::radians(degrees)),
		0,
		0,
		glm::sin(glm::radians(degrees)),
		glm::cos(glm::radians(degrees)),
		0,
		0,
		0,
		0,
		1,
		0,
		0,
		0,
		0,
		1);
}

glm::mat4 getTranslationMatrix(vec3 vect)
{
	glm::mat4 matrix(0);
	matrix[0][3] = vect.x;
	matrix[1][3] = vect.y;
	matrix[2][3] = vect.z;
	return matrix;
}

vector<vec3> generateLightOffsets()
{
	static vector<vec3> lightOffsets;
	if (lightOffsets.size() != 0)
	{
		return lightOffsets;
	}
	for (int i = 0; i < 20; i++)
	{
		float randx = (float) (rand()) / (float) (RAND_MAX);
		float randy = (float) (rand()) / (float) (RAND_MAX);
		float randz = (float) (rand()) / (float) (RAND_MAX);
		lightOffsets.push_back(vec3(randx, randy, randz));
	}
	return lightOffsets;
}

bool isInBounds(CanvasPoint point, glm::vec4 bounds)
{
	if (point.x < bounds.x)
		return false;
	if (point.x > bounds.z)
		return false;
	if (point.y < bounds.y)
		return false;
	if (point.y > bounds.w)
		return false;
	return true;
}

// Print a 4x4 matrix to stdout
void output(glm::mat4 matrix, std::string title)
{
	std::cout << title << ":\n";
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			std::cout << std::setw(3) << matrix[i][j] << " ";
		}
		std::cout << "\n";
	}
}

// Clip a vector of canvas points such that all points are within some bounds
vector<CanvasPoint>
bindToRectangle(vector<CanvasPoint> input, vec2 topLeft, vec2 bottomRight)
{
	vector<CanvasPoint> returnVector;
	for (CanvasPoint point : input)
	{
		if (topLeft.x > roundI(point.x))
			continue;
		if (bottomRight.x < roundI(point.x))
			continue;
		if (topLeft.y > roundI(point.y))
			continue;
		if (bottomRight.y < roundI(point.y))
			continue;
		returnVector.push_back(point);
	}
	return returnVector;
}

// Convert vector to uint32_t (assuming vector values range from 0 and 1)
uint32_t vec3ToColour(vec3 vect, int alpha)
{
	// Convert an RGB value and an alpha value to an int encoding them.
	uint32_t colour = (alpha << 24) + (uint8_t(vect.x) << 16) +
					  (uint8_t(vect.y) << 8) + uint8_t(vect.z);
	return colour;
}

// Convert RGB uint32_t to vector with values ranging from 0 - 255
vec3 intColToVec3(uint32_t c)
{
	return vec3((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
}

vector<float> interpolate(float from, float to, int steps)
{
	float diff = to - from;
	float interval = diff / steps;
	vector<float> interpolation;
	for (int i = 0; i < steps; i++)
	{
		interpolation.push_back(from + i * interval);
	}
	return interpolation;
}

vec3 vecMin(vec3 one, vec3 two)
{
	return vec3(
		one.x > two.x ? two.x : one.x,
		one.y > two.y ? two.y : one.y,
		one.z > two.z ? two.z : one.z);
}

vector<vec2> interpolate(vec2 from, vec2 to, int steps)
{
	vector<vec2> interpolation;
	if (steps < 3)
	{
		interpolation.push_back(from);
		interpolation.push_back(to);
		return interpolation;
	}
	vec2 diff = to - from;
	for (int i = 0; i <= steps; i++)
	{
		interpolation.push_back(from + (float(i) / steps) * diff);
	}
	return interpolation;
}

vector<vec3> interpolate(vec3 from, vec3 to, int steps)
{
	vector<vec3> interpolation;
	if (steps < 3)
	{
		interpolation.push_back(from);
		interpolation.push_back(to);
		return interpolation;
	}
	vec3 diff = to - from;
	for (int i = 0; i <= steps; i++)
	{
		interpolation.push_back(from + (float(i) / steps) * diff);
	}
	return interpolation;
}

vector<CanvasPoint> interpolate(CanvasPoint fromC, CanvasPoint toC, int steps)
{
	vector<CanvasPoint> interpolation;
	vec3 to(toC.x, toC.y, toC.depth);
	vec3 from(fromC.x, fromC.y, fromC.depth);
	vector<vec3> vecInterpolation = interpolate(from, to, steps);
	for (vec3 vect : vecInterpolation)
	{
		interpolation.push_back(
			CanvasPoint(roundI(vect.x), roundI(vect.y), vect.z));
	}
	return interpolation;
}

Translation::Translation(
	Animateable* object,
	std::function<vec3(vec3, int)> posFunc)
{
	target = object;
	origin = object->getPosition();
	f = posFunc;
	tick = 0;
}
// If animation is enabled, increase step and move the object.
void Translation::animate()
{
	if (!on)
		return;
	vec3 newPos = vec3(f(origin, tick));
	target->setPosition(newPos);
	tick++;
}
// Toggle animation between enabled/disabled
void Translation::toggle()
{
	on = !on;
}

Rotation::Rotation(Rotateable* object, vec3 degreesPerTick)
{
	target = object;
	startOrientation = object->getOrientation();
	dpt = degreesPerTick;
	tick = 0;
}
void Rotation::animate()
{
	if (!on)
		return;
	vec3 degrees = (float) tick * dpt;
	glm::mat3 newOrientation = glm::mat3(getZRotationMatrix(degrees.z)) *
							   glm::mat3(getYRotationMatrix(degrees.y)) *
							   glm::mat3(getXRotationMatrix(degrees.x)) *
							   startOrientation;
	target->setOrientation(newOrientation);
	tick++;
}
void Rotation::toggle()
{
	on = !on;
}

AdjustableRotation::AdjustableRotation(
	Rotateable* object,
	std::function<vec3(int)> degfunc)
{
	target = object;
	originalOrientation = object->getOrientation();
	f = degfunc;
	tick = 0;
}
void AdjustableRotation::animate()
{
	if (!on)
		return;
	vec3 degrees = f(tick);
	glm::mat3 newOrientation = glm::mat3(getZRotationMatrix(degrees.z)) *
							   glm::mat3(getYRotationMatrix(degrees.y)) *
							   glm::mat3(getXRotationMatrix(degrees.x)) *
							   originalOrientation;
	target->setOrientation(newOrientation);
	tick++;
}

void AdjustableRotation::toggle()
{
	on = !on;
}