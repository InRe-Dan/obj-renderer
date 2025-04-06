#pragma once

#include "DrawingWindow.h"
#include "Utils.h"
#include "TextureMap.h"
#include "ModelTriangle.h"


#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <functional>
#include <iomanip>


inline float quantize(float value, float level)
{
	return std::round(value * level) / level;
}

glm::mat4 getXRotationMatrix(float degrees);

glm::mat4 getYRotationMatrix(float degrees);

glm::mat4 getZRotationMatrix(float degrees);

glm::mat4 getTranslationMatrix(glm::vec3 vect);

std::vector<glm::vec3> generateLightOffsets();

inline glm::uint pack(const glm::vec4& rgba)
{
	return glm::packUnorm4x8(rgba);
}

inline glm::uint pack(const glm::vec3& rgb)
{
	return glm::packUnorm4x8(glm::vec4{ rgb, 1.0 });
}

inline glm::vec4 unpack(const glm::uint rgba)
{
	return glm::unpackUnorm4x8(rgba);
}

void bindToRectangle(std::vector<glm::vec2>& input, glm::vec2 topLeft, glm::vec2 bottomRight);

template<typename T>
std::vector<T> interpolate(const T& from, const T& to, size_t steps)
{
	std::vector<T> result;
	result.reserve(steps);
	for (size_t i = 0; i <= steps - 1; i++)
	{
		result.push_back(glm::mix(from, to, double(i) / steps));
	}
	return result;
}