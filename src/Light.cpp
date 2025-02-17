#include "Light.h"

#include "utility/vectors.h"

Light::Light(
	std::string n,
	glm::vec3 position,
	float strength,
	Colour colour,
	bool startState,
	bool isSoft,
	float radius)
{
	soft = isSoft;
	name = n;
	pos = position;
	str = strength;
	col = colour;
	r = col.red;
	g = col.green;
	b = col.blue;
	state = startState;
	this->radius = radius;
}

glm::vec3 Light::getPosition()
{
	return pos;
}

void Light::setPosition(glm::vec3 position)
{
	pos = position;
}