#include "Object.h"

#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <ModelTriangle.h>

using glm::vec4;
using std::array;
using std::string;
using std::vector;

Object::Object(string id)
{
	name = id;
}
void Object::setMaterial(string m)
{
	material = m;
}
void Object::translate(vec4 displacement)
{
	for (int i = 0; i < triangles.size(); i++)
	{
		triangles[i].vertices[0] += displacement;
		triangles[i].vertices[1] += displacement;
		triangles[i].vertices[2] += displacement;
	}
}