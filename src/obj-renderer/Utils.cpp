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

glm::vec3 parseTriplet(std::string_view str)
{
	std::vector<std::string_view> views = split(str, ' ');
	glm::vec3 vec{};
	std::from_chars(views[1].data(), views[1].data() + views[1].size(), vec.x);
	std::from_chars(views[2].data(), views[2].data() + views[2].size(), vec.y);
	std::from_chars(views[3].data(), views[3].data() + views[3].size(), vec.z);
	return vec;

}
