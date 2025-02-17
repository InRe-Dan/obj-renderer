#pragma once

#include "utility/vectors.h"

#include <vector>
#include <map>

#include <CanvasTriangle.h>
#include <TextureMap.h>

#include <glm/glm.hpp>


// Class to store information about a material in a MaterialFile. Shouldn't
// really be used alone.
class Material
{
  public:
	bool isTextured = false;
	bool hasNormalMap = false;
	bool isReflective = false;

	Material();

	Material(std::string name);

	void setDiffuseColour(glm::vec3 colour);

	void setAmbientColour(glm::vec3 colour);

	void setSpecularColour(glm::vec3 colour);

	void setSpecularExponent(float value);

	void setMap_Kd(std::string name);

	void setMap_Bump(std::string name);

	glm::vec3 getDiffuseColour();

	uint32_t getDiffuseColourInt();

	glm::vec3 getAmbientColour();

	glm::vec3 getSpecularColour();

	float getSpecularExponent();

	uint32_t getTexturePointColour(glm::vec2 uAndV);

	uint32_t getNormalMapRGB(glm::vec2 uAndV);
	glm::vec3 getNormalMapVector(glm::vec2 uAndV);
	void finishLoading();

	std::string materialName;

  private:
	bool diffuseDefined = false;
	glm::vec3 floatDiffuseColour = glm::vec3(1);
	uint32_t packedDiffuseRGB = 0xFFFFFFFF;
	bool ambientDefined = false;
	glm::vec3 floatAmbientColour = glm::vec3(1);
	uint32_t packedAmbientRGB = 0xFFFFFFFF;
	float specularDefined = false;
	glm::vec3 floatSpecularColour = glm::vec3(0);
	uint32_t packedSpecularRGB = 0xFFFFFFFF;
	float specularExponent = 0.0f;
	std::string map_Kd;
	std::string map_bump;
	TextureMap texture;
	TextureMap bump;
	std::vector<std::vector<glm::vec3>> bump_vectors;
};