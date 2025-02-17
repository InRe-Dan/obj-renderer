#pragma once

#include "Object.h"
#include "MaterialLib.h"

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
#include <iostream>
#include <unordered_map>

// Class to load and encapsulate .obj files and their .mtl files.
// Also used for basic manipulation of the geometry inside them.
class ObjectFile
{
  public:
	glm::vec3 position;
	glm::mat3 orientation;
	MaterialLib* matLib;

	ObjectFile(std::string filename, float scale);

	void printVertices();

	std::vector<Object> getObjects();

	Colour getKdOf(Object object);

	void translate(glm::vec4 displacement);

	void centerOn(glm::vec4 target);

  private:
	static glm::vec3 parseVertex(std::string input);

	static glm::vec2 parseTextureRatio(std::string input);

	static std::array<std::array<int, 3>, 3> parseFace(std::string input);

	std::string materialLib;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> vertexNormals;
	std::vector<glm::vec2> vertexTextureRatios;
	std::vector<std::array<std::array<int, 3>, 3>> faces;
	std::vector<Object> objects;
	std::string file;
	float scaleFactor;
};
