#include "TextureMap.h"

#include <filesystem>
#include <streambuf>
#include <array>

Surface::Surface(size_t width, size_t height, std::span<const glm::vec4> data)
	: width(width)
	, height(height)
	, data(data.begin(), data.end())
{}

Surface Surface::fromFile(std::filesystem::path& file)
{
	std::ifstream inputStream(file, std::ifstream::binary);
	std::stringstream buffer;
	buffer << inputStream.rdbuf();
	inputStream.close();
	std::string nextLine;
	
	// Get the "P6" magic number
	std::getline(buffer, nextLine);
	
	// Read the width and height line
	std::getline(buffer, nextLine);
	
	// Skip over any comment lines!
	while (nextLine.at(0) == '#') std::getline(buffer, nextLine);
	
	std::vector<std::string_view> widthAndHeight = split(nextLine, ' ');
	if (widthAndHeight.size() != 2)
		throw std::invalid_argument("Failed to parse width and height line, line was `" + nextLine + "`");

	// Read the max value (which we assume is 255)
	std::getline(buffer, nextLine);

	size_t width = std::stoi(std::string(widthAndHeight[0]));
	size_t height = std::stoi(std::string(widthAndHeight[1]));
	size_t length = width * height;
	std::vector<glm::vec4> data;
	data.reserve(length);
	
	std::string_view rest = buffer.view();
	for (size_t i = 0; i < length; i++) 
	{
		float r = std::bit_cast<uint8_t>(rest[i * 3 + 0]) / 255.f;
		float g = std::bit_cast<uint8_t>(rest[i * 3 + 1]) / 255.f;
		float b = std::bit_cast<uint8_t>(rest[i * 3 + 2]) / 255.f;
		
		data.emplace_back(r, g, b, 1.0);
	}

	return Surface(width, height, data);
}

glm::vec4 Surface::sample(glm::vec2 uv) const
{
	assert(isNormalized(uv.x) && isNormalized(uv.y));
	int sampleX = std::lround(uv.x * width);
	int sampleY = std::lround(uv.y * height);
	return data[sampleY * height + sampleX];
}
