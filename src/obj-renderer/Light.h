#pragma once

#include "vectors.h"

// Simple class to store light location and other attributes
class Light
{
  public:
	glm::vec3 pos = {0, 0, 0};
	glm::vec3 colour = { 1.0, 1.0, 1.0 };
	float strength = 1.0;
	/// Radius of soft lighting - zero for nonsoft.
	float radius = 0.0;
	bool on = true;

	friend auto operator<=>(const Light& a, const Light& b) = default;

};