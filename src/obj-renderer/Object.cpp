#include "Object.h"
#include "DrawingWindow.h"
#include "ModelTriangle.h"

#include <fstream>
#include <vector>
#include <string_view>

Object::Object(std::string_view id, std::shared_ptr<const Material> m)
	: name(id)
	, material(m)
{
	assert(material);
}