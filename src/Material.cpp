#include "Material.h"

#include "utility/vectors.h"

#include <vector>
#include <map>

#include "TextureMap.h"
#include "CanvasTriangle.h"
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

Material::Material(std::string_view name)
	:name(name) {}

void Material::setDiffuseColour(Colour colour)
{
	diffuse = colour;
}
void Material::setAmbientColour(Colour colour)
{
	ambient = colour;
}
void Material::setSpecularColour(Colour colour)
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
	bump_vectors = vector<vector<vec3>>();
	for (int i = 0; i < mine.getHeight(); i++)
	{
		bump_vectors.push_back(vector<vec3>());
		for (int j = 0; j < mine.getWidth(); j++)
		{
			Colour col = mine.getData()[i * mine.getWidth() + j];
			uint32_t integer = col.pack();
			glm::vec3 vector = vec3(
				(integer >> 16) & 0xFF,
				(integer >> 8) & 0xFF,
				(integer) & 0xFF);
			vec3 adjustedVector =
				glm::normalize(((vector / 255.0f) - 0.5f) * 2.0f);
			bump_vectors[i].push_back(adjustedVector);
		}
	}
}
std::optional<Colour> Material::getDiffuseColour() const
{
	return diffuse;
}

std::optional<Colour> Material::getAmbientColour() const
{
	return ambient;
}

std::optional<Colour> Material::getSpecularColour() const
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
