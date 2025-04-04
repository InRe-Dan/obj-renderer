#pragma once

#include <iostream>
#include <array>
#include <GLM/vec3.hpp>
#include <GLM/vec4.hpp>

/// Uses normalized RGB values.
class Colour : public glm::vec4 {
public:

	using glm::vec4::vec4;

	template<typename T>
		requires (std::tuple_size_v<T> == 3)
	Colour(const T& tupleLike) : Colour(std::get<0>(tupleLike), std::get<1>(tupleLike), std::get<2>(tupleLike), 1.0)
	{}

	float r() const { return x; }
	float& r() { return x; }
	float g() const { return y; }
	float& g() { return y; }
	float b() const { return z; }
	float& b() { return z; }
	float a() const { return w; }
	float& a() { return w; }

	/// Packs colour into an ARGB int
	uint32_t pack() const
	{
		uint32_t colour = 
			(static_cast<int8_t>(w * 255) << 24) 
			+ (static_cast<int8_t>(x * 255) << 16)
			+ (static_cast<int8_t>(y * 255) << 8)
			+ static_cast<int8_t>(z * 255);
		return colour;
	}
};

