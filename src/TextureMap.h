#pragma once

#include "Utils.h"
#include "Colour.h"

#include <string_view>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <span>

#include <GLM/vec2.hpp>


class Surface {
public:

	Surface(size_t width, size_t height, const std::span<Colour> data);

	/// Load from PPM file
	static Surface fromFile(std::filesystem::path& file);

	size_t getWidth() const { return width; }
	size_t getHeight() const { return height; }

	std::vector<Colour>& getData()
	{
		return data;
	}

	Colour sample(glm::vec2 uv) const;

private:
	size_t width;
	size_t height;
	std::vector<Colour> data;

};
