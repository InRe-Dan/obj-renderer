#include "Utils.h"

#include <algorithm>
#include <sstream>
#include <charconv>
#include <ranges>

std::vector<std::string_view> split(std::string_view str, char delimiter) {

	std::vector<std::string_view> tokens;
	for (const auto& v : std::ranges::split_view(str, delimiter))
	{
		tokens.emplace_back(v.begin(), v.end());
	}
	return tokens;
}

std::array<float, 3> parseTriplet(std::string_view str)
{
	std::vector<std::string_view> views = split(str, ' ');
	std::array<float, 3> arr;
	std::from_chars(views[1].data(), views[1].data() + views[1].size(), arr[0]);
	std::from_chars(views[2].data(), views[2].data() + views[2].size(), arr[1]);
	std::from_chars(views[3].data(), views[3].data() + views[3].size(), arr[2]);
	return arr;

}
