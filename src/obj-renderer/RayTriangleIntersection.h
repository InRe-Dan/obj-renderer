#pragma once

#include "ModelTriangle.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <iostream>

struct RayTriangleIntersection {
	/// nullptr only if there was no intersection
	const ModelTriangle* intersectedTriangle = nullptr;
	/// x: distance along the ray that the solution was found at
	/// y: ratio along the triangle's first edge that the solution is at (v1 - v0)
	/// z: ratio along the triangle's second edge that the solution is at (v2 - v0)
	glm::vec3 solution;

	glm::vec3 getPosition() const
	{
		const std::array<glm::vec3, 3>& vs = intersectedTriangle->vertices;
		glm::vec3 e1 = vs[1] - vs[0];
		glm::vec3 e2 = vs[2] - vs[0];
		return vs[0] + e1 * solution.y + e2 * solution.z;
	}
};
