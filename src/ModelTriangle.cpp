#include "ModelTriangle.h"
#include <utility>

#include "Object.h"
#include "TextureMap.h"
#include "Material.h"

using namespace glm;

ModelTriangle::ModelTriangle(
	std::array<glm::vec3, 3> vertices
	, std::array<glm::vec2, 3> texturePoints
	, const Object& parent)
	: vertices(vertices)
	, texturePoints(texturePoints)
	, parent(&parent)
{}

void ModelTriangle::setSmoothing(uint32_t smoothingGroup, std::array<glm::vec3, 3> vertexNormals)
{
	this->smoothingGroup = smoothingGroup;
	this->vertexNormals = vertexNormals;
}

glm::vec3 ModelTriangle::getNormal() const
{
	vec3 e0 = glm::normalize(vec3(vertices[0] - vertices[1]));
	vec3 e1 = glm::normalize(vec3(vertices[0] - vertices[2]));
	return glm::normalize(glm::cross(e0, e1));
}

glm::vec2 ModelTriangle::triangleToTexture(glm::vec2 UV) const
{
	vec2 e0 = texturePoints[1] - texturePoints[0];
	vec2 e1 = texturePoints[2] - texturePoints[0];
	vec2 texturePoint = texturePoints[0] + e0 * UV.x + e1 * UV.y;
	return texturePoint;
}

Colour ModelTriangle::sampleDiffuse(glm::vec2 triangleSpaceUV, bool texturesEnabled) const
{
	Colour col = parent->getMaterial().getDiffuseColour().value_or(Colour{ 0.5, 0.5, 0.5, 1.0 });
	if (const std::optional<Surface>& tex = parent->getMaterial().getDiffuseTexture();
		tex && texturesEnabled)
	{
		col = tex.value().sample(triangleToTexture({ triangleSpaceUV.x, triangleSpaceUV.y }));
	}
	return col;
}
