#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <array>
#include <assert.h>
#include <charconv>

std::vector<std::string_view> split(std::string_view str, char delimiter);

/// Parses strings such as "Kd 1.0, 3.5, -1.3" into float arrays
std::array<float, 3> parseTriplet(std::string_view str);

template<typename T>
T fromStr(std::string_view view)
{
	T val{};
	auto [_, err] = std::from_chars(view.data(), view.data() + view.size(), val);
	assert(err == std::errc());
	return val;
}

inline bool inRange(float x, float min, float max)
{
	return x >= min && x <= max;
}

inline bool isNormalized(float x)
{
	return inRange(x, 0.0, 1.0);
}
