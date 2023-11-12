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
#include <iomanip>   // std::setprecision, std::setw

using std::vector;
using glm::vec3;
using glm::vec2;
using std::string;

inline int roundI(float x) {
	return (x > 0)? int(x + 0.5) : int(x - 0.5);
}

string formatFloat(float num, int width) {
  string returnString;
  string floatString = std::to_string(num);
  bool foundPoint = false;
  int originalWidth = width;
  if (num >= 0.0f) {
    width--;
    returnString += " ";
  }
  for (char c : floatString) {
    if (c == '.') {
      foundPoint = true;
    }
    if (foundPoint && width == 0) break;
    returnString += c;
    width--;
  }
  if (width < 0) {
    return std::string(">", originalWidth);
  }
  for (int i = 0; i < width; i++) {
    returnString += "0";
  }
  return returnString;
}

string printVec(vec3 v) {
  return "(" + formatFloat(v.x, 5) + ", " + formatFloat(v.y, 5) + ", " + formatFloat(v.z, 5) + ")";
}

glm::mat4 getXRotationMatrix(float degrees) {
  return glm::mat4(
    1, 0, 0, 0,
    0, glm::cos(glm::radians(degrees)), - glm::sin(glm::radians(degrees)), 0, 
    0, glm::sin(glm::radians(degrees)), glm::cos(glm::radians(degrees)), 0,
    0, 0, 0, 1
  );
}

glm::mat4 getYRotationMatrix(float degrees) {
  return glm::mat4(
    glm::cos(glm::radians(degrees)), 0, glm::sin(glm::radians(degrees)), 0, 
    0, 1, 0, 0, 
    -glm::sin(glm::radians(degrees)), 0, glm::cos(glm::radians(degrees)), 0,
    0, 0, 0, 1
  );
}

glm::mat4 getZRotationMatrix(float degrees) {
  return glm::mat4(
    glm::cos(glm::radians(degrees)), -glm::sin(glm::radians(degrees)), 0, 0,
    glm::sin(glm::radians(degrees)), glm::cos(glm::radians(degrees)), 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  );
}

glm::mat4 getTranslationMatrix(vec3 vect) {
  glm::mat4 matrix(0);
  matrix[0][3] = vect.x;
  matrix[1][3] = vect.y;
  matrix[2][3] = vect.z;
  return matrix;
}

bool isInBounds(CanvasPoint point, vec4 bounds) {
  if (point.x < bounds.x) return false;
  if (point.x > bounds.z) return false;
  if (point.y < bounds.y) return false;
  if (point.y > bounds.w) return false;
  return true;
}

void output(glm::mat4 matrix, std::string title) {
  std::cout << title << ":\n";
  for (int i = 0; i < 4; i++ ) {
    for (int j = 0; j < 4; j++) {
      std::cout << std::setw(3) << matrix[i][j] << " ";
    }
    std::cout << "\n";
  }
}

vector<CanvasPoint> bindToRectangle(vector<CanvasPoint> input, vec2 topLeft, vec2 bottomRight) {
  vector<CanvasPoint> returnVector;
  for (CanvasPoint point : input) {
    if (topLeft.x > roundI(point.x)) continue;
    if (bottomRight.x < roundI(point.x)) continue;
    if (topLeft.y > roundI(point.y)) continue;
    if (bottomRight.y < roundI(point.y)) continue;
    returnVector.push_back(point);
  }
  return returnVector;
}

uint32_t vec3ToColour(vec3 vect, int alpha) {
	// Convert an RGB value and an alpha value to an int encoding them.
	uint32_t colour = (alpha << 24) + (uint8_t(vect.x) << 16) + (uint8_t(vect.y) << 8) + uint8_t(vect.z);
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
	vector<vec2> interpolation;
  if (steps < 3) {
    interpolation.push_back(from);
    interpolation.push_back(to);
    return interpolation;
  }
  vec2 diff = to - from;
	for (int i = 0; i <= steps; i++) {
		interpolation.push_back(from + (float(i) / steps) * diff);
	}
	return interpolation;
}

vector<vec3> interpolate(vec3 from, vec3 to, int steps) {
	vector<vec3> interpolation;
  if (steps < 3) {
    interpolation.push_back(from);
    interpolation.push_back(to);
    return interpolation;
  }
  vec3 diff = to - from;
	for (int i = 0; i <= steps; i++) {
		interpolation.push_back(from + (float(i) / steps) * diff);
	}
	return interpolation;
}

vector<CanvasPoint> interpolate(CanvasPoint fromC, CanvasPoint toC, int steps) {
  vector<CanvasPoint> interpolation;
  vec3 to(toC.x, toC.y, toC.depth);
  vec3 from(fromC.x, fromC.y, fromC.depth);
  vector<vec3> vecInterpolation = interpolate(from, to, steps);
	for (vec3 vect : vecInterpolation) {
		interpolation.push_back(CanvasPoint(roundI(vect.x), roundI(vect.y), vect.z));
	}
	return interpolation;
}
