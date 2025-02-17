#include "ModelTriangle.h"
#include <utility>

using namespace glm;

ModelTriangle::ModelTriangle() {
	vertices = {vec4(), vec4(), vec4()};
	texturePoints = {vec2(), vec2(), vec2()};
	colour = Colour(0, 0, 0);
	normal = vec3(0);
	material = NULL;
	smoothingGroup = 0;
	vertexNormals = {vec3(), vec3(), vec3()};
	hasVertexNormals = false;
}

ModelTriangle::ModelTriangle(std::array<glm::vec4, 3> vs, std::array<vec2, 3> ts, Colour c, glm::vec3 n, Material *m, uint32_t sG, std::array<glm::vec3, 3> vNs, bool hasVNs) {
	vertices = vs;
	texturePoints = ts;
	colour = c;
	normal = n;
	material = m;
	smoothingGroup = sG;
	vertexNormals = vNs;
	hasVertexNormals = hasVNs;
}

std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle) {
	os << "(" << triangle.vertices[0].x << ", " << triangle.vertices[0].y << ", " << triangle.vertices[0].z << ")\n";
	os << "(" << triangle.vertices[1].x << ", " << triangle.vertices[1].y << ", " << triangle.vertices[1].z << ")\n";
	os << "(" << triangle.vertices[2].x << ", " << triangle.vertices[2].y << ", " << triangle.vertices[2].z << ")\n";
	return os;
}
