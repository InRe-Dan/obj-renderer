#pragma once

#include "ModelTriangle.h"

#include <GLM/vec3.hpp>
#include <GLM/vec2.hpp>
#include <iostream>

struct RayTriangleIntersection {
	/// nullptr only if there was no intersection
	const ModelTriangle* intersectedTriangle = nullptr;
	/// x: distance along the ray that the solution was found at
	/// y: ratio along the triangle's first edge that the solution is at (v1 - v0)
	/// z: ratio along the triangle's second edge that the solution is at (v2 - v0)
	glm::vec3 solution;
};
