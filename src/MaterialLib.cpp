#include "MaterialLib.h"

#include "Material.h"

#include "CanvasTriangle.h"
#include "DrawingWindow.h"
#include "CanvasPoint.h"
#include "CanvasTriangle.h"
#include "Colour.h"
#include "Utils.h"
#include "TextureMap.h"
#include "ModelTriangle.h"

#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <unordered_map>
#include <charconv>
#include <string_view>
#include <filesystem>

using glm::vec2;
using glm::vec3;
using std::atoi;
using std::ifstream;
using std::stof;
using std::string;
using std::vector;

MaterialLib::MaterialLib()
{
	materials.emplace("default", std::make_shared<Material>());
}

MaterialLib::MaterialLib(const std::filesystem::path& filename)
{
	materials.emplace("default", std::make_shared<Material>());
	ifstream inputStream;
	inputStream.open(filename, std::ios::in);
	std::stringstream buffer;
	buffer << inputStream.rdbuf();
	inputStream.close();
	string line, code, materialName;
	while (std::getline(buffer, line))
	{
		if (line.empty())
		{
			continue;
		}
		std::string_view code = split(line, ' ').at(0);
		if (code.compare("newmtl") == 0)
		{
			materialName = split(line, ' ').at(1);
			materials.emplace(materialName, std::make_shared<Material>(materialName));
		}
		else if (code.compare("Kd") == 0)
		{
			materials[materialName]->setDiffuseColour(Colour(parseTriplet(line)));
		}
		else if (code.compare("Ka") == 0)
		{
			materials[materialName]->setAmbientColour(Colour(parseTriplet(line)));
		}
		else if (code.compare("Ks") == 0)
		{
			materials[materialName]->setSpecularColour(Colour(parseTriplet(line)));
		}
		else if (code.compare("Ns") == 0)
		{
			vector<std::string_view> lineSplit = split(line, ' ');
			float exponent(stof(std::string(lineSplit.at(1)).data()));
			materials[materialName]->setSpecularExponent(exponent);
		}
		else if (code.compare("map_Kd") == 0)
		{
			vector<std::string_view> lineSplit = split(line, ' ');
			materials[materialName]->setDiffuseMap(
				Surface::fromFile(std::filesystem::path("assets/texture").append(lineSplit[1]))
			);
		}
		else if (code.compare("map_bump") == 0)
		{
			vector<std::string_view> lineSplit = split(line, ' ');
			materials[materialName]->setBumpMap(
				Surface::fromFile(std::filesystem::path("assets/normal").append(lineSplit[1]))
			);
		}
		else if (code.compare("illum") == 0)
		{
			vector<std::string_view> lineSplit = split(line, ' ');
			if (lineSplit.at(1).compare("2") == 0)
				materials[materialName]->setReflectivity(1.0);
		}
	}

}