#pragma once

#include <glm/glm.hpp>
#include <string>
#include <array>
#include "Colour.h"

using glm::vec4;
using glm::vec2;

class Material;

struct ModelTriangle {
	std::array<glm::vec4, 3> vertices{};
	std::array<vec2, 3> texturePoints{};
	Colour colour{};
	glm::vec3 normal{};
	Material *material;

	ModelTriangle();
	ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, Colour trigColour);
	friend std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle);
};
