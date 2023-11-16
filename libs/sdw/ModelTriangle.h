#pragma once

#include <glm/glm.hpp>
#include <string>
#include <array>
#include "Colour.h"

using glm::vec4;
using glm::vec3;
using glm::vec2;

class Material;

struct ModelTriangle {
	std::array<glm::vec4, 3> vertices;
	std::array<vec2, 3> texturePoints;
	Colour colour;
	glm::vec3 normal;
	Material *material;
	uint32_t smoothingGroup;
	std::array<glm::vec3, 3> vertexNormals;
	bool hasVertexNormals;

	ModelTriangle();
	ModelTriangle(std::array<glm::vec4, 3> vs, std::array<vec2, 3> ts, Colour c, glm::vec3 n, Material *m, uint32_t sG, std::array<glm::vec3, 3> vNs, bool hasVertexNormals);
	friend std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle);
};
