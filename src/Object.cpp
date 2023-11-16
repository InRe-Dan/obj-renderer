#pragma once

#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <ModelTriangle.h>

using std::string;
using std::vector;
using std::array;
using glm::vec4;

// Class to store basic attributes about objects in ObjectFiles.
class Object {
	public:
	Object(string id) {
		name = id;
	}
	void setMaterial(string m) {
		std::cout << m << std::endl;
		material = m;
	}
  void translate(vec4 displacement) {
    for (int i = 0; i < triangles.size(); i++) {
      triangles[i].vertices[0] += displacement;
      triangles[i].vertices[1] += displacement;
      triangles[i].vertices[2] += displacement;

    }
  }
	vector<ModelTriangle> triangles;
	string material;
	string name;
};