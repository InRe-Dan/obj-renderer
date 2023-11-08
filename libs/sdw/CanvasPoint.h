#pragma once

#include <iostream>
#include <glm/glm.hpp>

struct CanvasPoint {
	float x{};
	float y{};
	float depth{};
	float brightness{};
	glm::vec2 texturePoint{};

	CanvasPoint();
	CanvasPoint(float xPos, float yPos);
	CanvasPoint(float xPos, float yPos, float pointDepth);
	CanvasPoint(float xPos, float yPos, float pointDepth, float pointBrightness);
	friend std::ostream &operator<<(std::ostream &os, const CanvasPoint &point);
};
