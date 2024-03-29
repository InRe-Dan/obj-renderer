#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include "ModelTriangle.h"

struct RayTriangleIntersection {
	glm::vec3 intersectionPoint;
	float distanceFromCamera;
	ModelTriangle intersectedTriangle;
	int triangleIndex;
	glm::vec3 normal;

	RayTriangleIntersection();
	RayTriangleIntersection(const glm::vec3 &point, float distance, const ModelTriangle &triangle, int index, glm::vec3 normalVec);
	friend std::ostream &operator<<(std::ostream &os, const RayTriangleIntersection &intersection);
};
