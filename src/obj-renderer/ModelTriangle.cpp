#include <utility>

#include "ModelTriangle.h"
#include "Object.h"
#include "TextureMap.h"
#include "Material.h"

#include <glm/vec4.hpp>

ModelTriangle::ModelTriangle(
	std::array<glm::vec3, 3> vertices
	, std::array<glm::vec2, 3> texturePoints
	, const Object& parent)
	: vertices(vertices)
	, texturePoints(texturePoints)
	, parent(&parent)
{}

void ModelTriangle::setSmoothing(uint32_t newGroup, std::array<glm::vec3, 3> newNormals)
{
	this->smoothingGroup = newGroup;
	this->vertexNormals = newNormals;
}

glm::vec2 ModelTriangle::triangleToTexture(glm::vec2 UV) const
{
	glm::vec2 e0 = texturePoints[1] - texturePoints[0];
	glm::vec2 e1 = texturePoints[2] - texturePoints[0];
	glm::vec2 texturePoint = texturePoints[0] + e0 * UV.x + e1 * UV.y;
	return texturePoint;
}

glm::vec4 ModelTriangle::sampleDiffuse(glm::vec2 triangleSpaceUV, bool texturesEnabled) const
{
	glm::vec4 col = parent->getMaterial().getDiffuseColour().value_or(glm::vec4{ 0.5, 0.5, 0.5, 1.0 });
	if (const std::optional<Surface>& tex = parent->getMaterial().getDiffuseTexture();
		tex && texturesEnabled)
	{
		col = tex.value().sample(triangleToTexture({ triangleSpaceUV.x, triangleSpaceUV.y }));
	}
	return col;
}
