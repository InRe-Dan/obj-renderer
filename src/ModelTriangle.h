#pragma once

#include <GLM/vec2.hpp>
#include <GLM/vec3.hpp>
#include <string>
#include <array>
#include <optional>
#include "Colour.h"

class Material;
class Object;

struct ModelTriangle {
	/// Clockwise-wound
	std::array<glm::vec3, 3> vertices{};
	std::array<glm::vec2, 3> texturePoints{};
	uint32_t smoothingGroup = 0;
	/// If smoothing group is anything other than 0, it's assumed that normals are correctly set.
	std::array<glm::vec3, 3> vertexNormals;
	/// Never null
	const Object* parent = nullptr;

	ModelTriangle(
		std::array<glm::vec3, 3> vertices
		, std::array<glm::vec2, 3> texturePoints
		, const Object& parent);

	void setSmoothing(uint32_t smoothingGroup, std::array<glm::vec3, 3> vertexNormals);

	/// Gets the normal based on vertices and winding.
	glm::vec3 getNormal() const;

	/// Map a triangle-space UV coordinate to one on the target material's texture space.
	glm::vec2 triangleToTexture(glm::vec2 UV) const;

	Colour sampleDiffuse(glm::vec2 triangleSpaceUV, bool texturesEnabled) const;

};
