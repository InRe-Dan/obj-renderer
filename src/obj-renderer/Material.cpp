#include "Material.h"

#include "vectors.h"
#include "TextureMap.h"

#include <vector>
#include <map>

#include <glm/glm.hpp>


// Class to store information about a material in a MaterialFile. Shouldn't
// really be used alone.

Material::Material(std::string_view name)
	:name(name) {}

void Material::setDiffuseColour(glm::vec4 colour)
{
	diffuse = colour;
}
void Material::setAmbientColour(glm::vec4 colour)
{
	ambient = colour;
}
void Material::setSpecularColour(glm::vec4 colour)
{
	specular = colour;
}
void Material::setSpecularExponent(float value)
{
	specularExponent = value;
}
void Material::setDiffuseMap(const Surface& map)
{
	diffuseTexture = map;
}

void Material::setBumpMap(const Surface& map)
{
	normalTexture = map;
	Surface& mine = normalTexture.value();
	// Load as a "texturemap"
	bump_vectors = std::vector<std::vector<glm::vec3>>();
	for (int i = 0; i < mine.getHeight(); i++)
	{
		bump_vectors.push_back(std::vector<glm::vec3>());
		for (int j = 0; j < mine.getWidth(); j++)
		{
			glm::vec4 col = mine.getData()[i * mine.getWidth() + j];
			col = (col - 0.5f) * 2.0f;
			bump_vectors[i].push_back(col);
		}
	}
}
std::optional<glm::vec4> Material::getDiffuseColour() const
{
	return diffuse;
}

std::optional<glm::vec4> Material::getAmbientColour() const
{
	return ambient;
}

std::optional<glm::vec4> Material::getSpecularColour() const
{
	return specular;
}

float Material::getSpecularExponent() const
{
	return specularExponent;
}

const std::optional<Surface>& Material::getDiffuseTexture() const
{
	return diffuseTexture;
}

const std::optional<Surface>& Material::getNormalTexture() const
{
	return normalTexture;
}
