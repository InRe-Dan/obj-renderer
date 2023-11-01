#pragma once

#include <CanvasTriangle.h>
#include <vector>
#include <glm/glm.hpp>

using std::vector;
using glm::vec3;
using glm::vec2;
using glm::round;
using std::string;
using std::ifstream;
using std::stof;
using std::atoi;
using std::cout;

class Material {
	public:
		Material(string name="UNSET") {
			materialName = name;
		}
		void setDiffuseColour(vec3 colour) {
			floatDiffuseColour = colour;
			packedDiffuseRGB = 255 << 24;
			packedDiffuseRGB += int(colour.x * 255) << 16;
			packedDiffuseRGB += int(colour.y * 255) << 8;
			packedDiffuseRGB += int(colour.z * 255);
		}
		uint32_t getDiffuseColour() {
			return packedDiffuseRGB;
		}
	string materialName;
	private:
		vec3 floatDiffuseColour;
		uint32_t packedDiffuseRGB = 0;
};