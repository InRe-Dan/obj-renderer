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

using std::vector;
using glm::vec3;
using glm::vec2;
using glm::round;

uint32_t vec3ToColour(vec3 vect, int alpha) {
	// Convert an RGB value and an alpha value to an int encoding them.
	uint32_t colour = (alpha << 24) + (int(vect.x) << 16) + (int(vect.y) << 8) + int(vect.z);
	return colour; 
}


vector<float> interpolate(float from, float to, int steps) {
	float diff = to - from;
	float interval = diff / steps;
	vector<float> interpolation;
	for (int i = 0; i < steps; i++) {
		interpolation.push_back(from + i * interval);
	}
	return interpolation;
}

vector<vec2> interpolate(vec2 from, vec2 to, int steps) {
	if (steps == 1) {vector<vec2> vec; vec.push_back(from); return vec;};
	if (steps == 1) {vector<vec2> vec; vec.push_back(from); vec.push_back(to); return vec;};
	vec2 diff = to - from;
	vec2 interval = diff * float(1) / float(steps);
	vector<vec2> interpolation;
	for (int i = 0; i < steps; i++) {
		interpolation.push_back(from + float(i) * interval);
	}
	return interpolation;
}

vector<vec2> interpolate(CanvasPoint from, CanvasPoint to, int steps) {
	vec2 fromP(from.x, from.y);
	vec2 toP(to.x, to.y);
	return interpolate(fromP, toP, steps);
}
vector<vec2> interpolate(TexturePoint from, TexturePoint to, int steps) {
	vec2 fromP(from.x, from.y);
	vec2 toP(to.x, to.y);
	return interpolate(fromP, toP, steps);
}


vector<vec3> interpolate(vec3 from, vec3 to, int steps) {
	vec3 diff = to - from;
	vec3 interval = diff * float(1) / float(steps);
	vector<vec3> interpolation;
	for (int i = 0; i < steps; i++) {
		interpolation.push_back(from + float(i) * interval);
	}
	return interpolation;
}

vector<ModelTriangle> readOBJ(const char *filename, float scale) {
	std::ifstream inputStream;
	inputStream.open(filename, std::ios::in);
	vector<vec3> vertices;
	vector<ModelTriangle> triangles;
	std::string line;
	for (int lines = 0; std::getline(inputStream,line); lines++) {
		std::cout << line << '\n';
	}
	inputStream.close();
	return triangles;
}
