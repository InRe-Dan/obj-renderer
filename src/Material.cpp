#include "Material.h"

#include "utility/vectors.h"

#include <vector>
#include <map>

#include <TextureMap.h>
#include <CanvasTriangle.h>
#include <glm/glm.hpp>

using glm::vec2;
using glm::vec3;
using std::atoi;
using std::cout;
using std::ifstream;
using std::stof;
using std::string;
using std::vector;

// Class to store information about a material in a MaterialFile. Shouldn't
// really be used alone.

Material::Material()
{
	materialName = "default";
}
Material::Material(string name)
{
	materialName = name;
}
void Material::setDiffuseColour(vec3 colour)
{
	diffuseDefined = true;
	floatDiffuseColour = colour;
	packedDiffuseRGB = 0;
	packedDiffuseRGB = 255 << 24;
	packedDiffuseRGB += int(colour.x * 255) << 16;
	packedDiffuseRGB += int(colour.y * 255) << 8;
	packedDiffuseRGB += int(colour.z * 255);
}
void Material::setAmbientColour(vec3 colour)
{
	ambientDefined = true;
	floatAmbientColour = colour;
	packedAmbientRGB = 0;
	packedAmbientRGB = 255 << 24;
	packedAmbientRGB += int(colour.x * 255) << 16;
	packedAmbientRGB += int(colour.y * 255) << 8;
	packedAmbientRGB += int(colour.z * 255);
}
void Material::setSpecularColour(vec3 colour)
{
	specularDefined = true;
	floatSpecularColour = colour;
	packedSpecularRGB = 0;
	packedSpecularRGB = 255 << 24;
	packedSpecularRGB += int(colour.x * 255) << 16;
	packedSpecularRGB += int(colour.y * 255) << 8;
	packedSpecularRGB += int(colour.z * 255);
}
void Material::setSpecularExponent(float value)
{
	specularExponent = value;
}
void Material::setMap_Kd(string name)
{
	map_Kd = name;
	texture = TextureMap("assets/texture/" + name);
	isTextured = true;
}
void Material::setMap_Bump(string name)
{
	map_bump = name;
	// Load as a "texturemap"
	bump = TextureMap("assets/normal/" + name);
	hasNormalMap = true;
	bump_vectors = vector<vector<vec3>>();
	for (int i = 0; i < bump.height; i++)
	{
		bump_vectors.push_back(vector<vec3>());
		for (int j = 0; j < bump.width; j++)
		{
			uint32_t integer = bump.pixels[i * bump.width + j];
			vec3 vector = vec3(
				(integer >> 16) & 0xFF,
				(integer >> 8) & 0xFF,
				(integer) & 0xFF);
			vec3 adjustedVector =
				glm::normalize(((vector / 255.0f) - 0.5f) * 2.0f);
			bump_vectors[i].push_back(adjustedVector);
		}
	}
}
vec3 Material::getDiffuseColour()
{
	return floatDiffuseColour;
}
uint32_t Material::getDiffuseColourInt()
{
	return packedDiffuseRGB;
}
vec3 Material::getAmbientColour()
{
	return floatAmbientColour;
}
vec3 Material::getSpecularColour()
{
	return floatSpecularColour;
}
float Material::getSpecularExponent()
{
	return specularExponent;
}
uint32_t Material::getTexturePointColour(vec2 uAndV)
{
	// cout << uAndV.x * texture.height << " " << uAndV.y * texture.width <<
	// "\n";
	return texture.pixels
		[roundI(uAndV.x * (texture.height - 1)) * texture.width +
		 roundI(uAndV.y * (texture.width - 1))];
	// upscaledFrameBuffer[i][j] = cobbles.pixels[i * cobbles.width + j];
}
uint32_t Material::getNormalMapRGB(vec2 uAndV)
{
	// cout << uAndV.x * texture.height << " " << uAndV.y * texture.width <<
	// "\n";
	return texture.pixels
		[roundI(uAndV.x * (texture.height - 1)) * texture.width +
		 roundI(uAndV.y * (texture.width - 1))];
	// upscaledFrameBuffer[i][j] = cobbles.pixels[i * cobbles.width + j];
}
vec3 Material::getNormalMapVector(vec2 uAndV)
{
	return bump_vectors.at(roundI((uAndV.x * (bump.height - 1))))
		.at(roundI((uAndV.y * (bump.width - 1))));
	// upscaledFrameBuffer[i][j] = cobbles.pixels[i * cobbles.width + j];
}
void Material::finishLoading()
{
	if (!diffuseDefined)
	{
		cout << "\nERROR!! A material was defined without a diffuse colour!"
			 << std::endl;
		exit(1);
	}
	if (!ambientDefined)
	{
		floatAmbientColour = floatDiffuseColour;
		packedAmbientRGB = packedDiffuseRGB;
	}
	if (!specularDefined)
	{
		floatSpecularColour = floatDiffuseColour;
		packedSpecularRGB = packedDiffuseRGB;
	}
}