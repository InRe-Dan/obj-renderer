#include "ObjectFile.h"

#include "Object.h"
#include "MaterialLib.h"

#include "CanvasTriangle.h"
#include "DrawingWindow.h"
#include "CanvasPoint.h"
#include "CanvasTriangle.h"
#include "Colour.h"
#include "Utils.h"
#include "TextureMap.h"
#include "ModelTriangle.h"

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <charconv>
#include <filesystem>
#include <GLM/vec2.hpp>
#include <GLM/vec3.hpp>
#include <GLM/vec4.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;

using std::atoi;
using std::cout;
using std::ifstream;
using std::stof;
using std::string;
using std::vector;

ObjectFile::ObjectFile(const std::filesystem::path& filePath)
{
	assert(std::filesystem::status(filePath).type() == std::filesystem::file_type::regular);
	ifstream inputStream(filePath, std::ios::in);
	string line;
	// Default object, material library and material in case .obj does not define them
	objects.push_back(std::make_unique<Object>("unnamed", matLib.getDefault()));
	uint32_t smoothingGroup = 0;
	while (std::getline(inputStream, line))
	{
		std::vector<std::string_view> tokens = split(line, ' ');
		if (tokens.empty())
		{
			continue;
		}
		std::string_view code = split(line, ' ').at(0);
		if (code.compare("mtllib") == 0)
		{
			std::string_view matlibname = split(line, ' ').at(1);
			matLib = MaterialLib(std::filesystem::path() / "assets" / "mtl" / matlibname);
		}
		else if (code.compare("o") == 0)
		{
			objects.push_back(std::make_unique<Object>(split(line, ' ').at(1), matLib.getDefault()));
		}
		else if (code.compare("usemtl") == 0)
		{
			objects.back()->setMaterial(matLib.get(split(line, ' ').at(1)));
		}
		else if (code.compare("v") == 0)
		{
			vec3 vertex = parseVertex(line);
			vertices.push_back(vertex);
		}
		else if (code.compare("s") == 0)
		{
			std::string_view option = split(line, ' ').at(1);
			if (code.compare("off") == 0)
				smoothingGroup = 0;
			else
				smoothingGroup = fromStr<int>(option);
		}
		else if (code.compare("vn") == 0)
		{
			vec3 vertexNormal = parseVertex(line);
			vertexNormals.push_back(glm::normalize(vertexNormal));
		}
		else if (code.compare("vt") == 0)
		{
			vec2 vertexTextureRatio = parseTextureRatio(line);
			vertexTextureRatios.push_back(vertexTextureRatio);
		}
		else if (code.compare("f") == 0)
		{
			std::array<std::array<int, 3>, 3> face = parseFace(line);
			std::array<vec3, 3> faceVertices
			{
				vec3(vertices.at(face[0][0] - 1)),
				vec3(vertices.at(face[1][0] - 1)),
				vec3(vertices.at(face[2][0] - 1))
			};

			Colour colour = objects.back()->getMaterial().getDiffuseColour().value();
			std::array<vec2, 3> ts = { vec2(0), vec2(0), vec2(0) };
			if (face[0][1] > 0)
			{
				ts = std::array<vec2, 3>{
					vertexTextureRatios.at(face[0][1] - 1),
						vertexTextureRatios.at(face[1][1] - 1),
						vertexTextureRatios.at(face[2][1] - 1),
				};
			}
			std::optional<std::array<glm::vec3, 3>> triangleVertNormals;
			if (face[0][2] > 0)
			{
				triangleVertNormals = std::array<vec3, 3>
				{
					vertexNormals.at(face[0][2] - 1),
						vertexNormals.at(face[1][2] - 1),
						vertexNormals.at(face[2][2] - 1)
				};
			}
			uint32_t sG = smoothingGroup;
			vec3 e0 = glm::normalize(vec3(faceVertices[0] - faceVertices[1]));
			vec3 e1 = glm::normalize(vec3(faceVertices[0] - faceVertices[2]));
			vec3 normal = glm::normalize(glm::cross(e0, e1));
			objects.back()->getTris().push_back(
				ModelTriangle(faceVertices, ts, *objects.back()));
			if (smoothingGroup && triangleVertNormals)
			{
				objects.back()->getTris().back().setSmoothing(smoothingGroup, triangleVertNormals.value());
			}
			faces.push_back(face);
		}
	}
	inputStream.close();
}

vector<std::unique_ptr<Object>>& ObjectFile::getObjects()
{
	return objects;
}

vec3 ObjectFile::parseVertex(std::string_view input)
{
	vector<std::string_view> splitStr = split(input, ' ');
	return
	{
		fromStr<float>(splitStr[1]),
		fromStr<float>(splitStr[2]),
		fromStr<float>(splitStr[3])
	};
}

vec2 ObjectFile::parseTextureRatio(std::string_view input)
{
	vector<std::string_view> splitStr = split(input, ' ');
	return { fromStr<float>(splitStr.at(1)), fromStr<float>(splitStr.at(2)) };
}

std::array<std::array<int, 3>, 3> ObjectFile::parseFace(std::string input)
{
	// vertex/texture/normal format
	std::array<std::array<int, 3>, 3> faceData;
	std::vector<std::string_view> tokens = split(input, ' ');
	for (int i = 1; i < 4; i++)
	{
		std::vector<std::string_view> information = split(tokens[i], '/');
		std::string_view locationIndex = information.at(0);
		std::optional<std::string_view> normalIndex;
		std::optional<std::string_view> textureIndex;
		if (information.size() > 1)
		{
			if (!information.at(1).empty())
			{
				textureIndex = information.at(1);
			}
		}
		if (information.size() > 2)
		{
			if (!information.at(2).empty())
			{
				normalIndex = information.at(2);
			}
		}
		faceData[i - 1] = std::array<int, 3>{
			fromStr<int>(locationIndex),
				fromStr<int>(textureIndex.value_or("-1")),
				fromStr<int>(normalIndex.value_or("-1"))
		};
	}
	return faceData;
}