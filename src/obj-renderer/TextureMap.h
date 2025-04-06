#pragma once

#include "Utils.h"

#include <string_view>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <span>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>


class Surface {
public:

	Surface(size_t width, size_t height, std::span<const glm::vec4> data);

	/// Load from PPM file
	static Surface fromFile(std::filesystem::path& file);

	size_t getWidth() const { return width; }
	size_t getHeight() const { return height; }

	std::vector<glm::vec4>& getData()
	{
		return data;
	}

	glm::vec4 sample(glm::vec2 uv) const;

private:
	size_t width;
	size_t height;
	std::vector<glm::vec4> data;

};
