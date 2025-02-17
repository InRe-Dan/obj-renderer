#pragma once

#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <ModelTriangle.h>

// Class to store basic attributes about objects in ObjectFiles.
class Object
{
  public:
	Object(std::string id);
	void setMaterial(std::string m);
	void translate(glm::vec4 displacement);
	std::vector<ModelTriangle> triangles;
	std::string material;
	std::string name;
};