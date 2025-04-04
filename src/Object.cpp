#include "Object.h"

#include "CanvasTriangle.h"
#include "DrawingWindow.h"
#include "CanvasPoint.h"
#include "CanvasTriangle.h"
#include "ModelTriangle.h"

#include <fstream>
#include <vector>
#include <string_view>
#include <glm/glm.hpp>

using glm::vec4;
using std::array;
using std::string;
using std::vector;

Object::Object(std::string_view id, std::shared_ptr<const Material> m)
	: name(id)
	, material(m)
{
	assert(material);
}