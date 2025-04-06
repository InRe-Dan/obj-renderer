#pragma once

#include "Object.h"
#include "MaterialLib.h"

#include "DrawingWindow.h"
#include "Utils.h"
#include "TextureMap.h"
#include "ModelTriangle.h"


#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <filesystem>

// Class to load and encapsulate .obj files and their .mtl files.
// Shouldn't be copied around, as there are many parent-child relationships set up.
class ObjectFile
{
  public:

	ObjectFile(const std::filesystem::path& filePath);

	ObjectFile(const ObjectFile&) = delete;
	ObjectFile(ObjectFile&&) = delete;
	ObjectFile& operator=(const ObjectFile&) = delete;
	ObjectFile& operator=(ObjectFile&&) = delete;

	std::vector<std::unique_ptr<Object>>& getObjects();

  private:
	MaterialLib matLib;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> vertexNormals;
	std::vector<glm::vec2> vertexTextureRatios;
	std::vector<std::array<std::array<int, 3>, 3>> faces;
	std::vector<std::unique_ptr<Object>> objects;
	std::string file;
};
