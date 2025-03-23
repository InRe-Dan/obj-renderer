#pragma once

#include "utility/vectors.h"

// Simple class to store light location and other attributes
class Light : public Animateable
{
  public:
	Light(
		std::string n,
		glm::vec3 position,
		float strength,
		Colour colour,
		bool startState,
		bool isSoft = false,
		float radius = 0.05);

	glm::vec3 getPosition();

	void setPosition(glm::vec3 position);

	std::string name;
	glm::vec3 pos;
	float str;
	int r;
	int g;
	int b;
	Colour col;
	bool state;
	bool soft;
	float radius;
};