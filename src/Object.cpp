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

class Object {
	public:
	Object(string id) {
		name = id;
	}
	void setMaterial(string m) {
		material = m;
	}
	vector<ModelTriangle> triangles;
	string material;
	string name;
};