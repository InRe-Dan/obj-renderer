#pragma once

#include <CanvasTriangle.h>
#include <vector>
#include <glm/glm.hpp>
#include <TextureMap.h>
#include <map>

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
    bool isTextured = false;
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
    void setMap_Kd(string name) {
      map_Kd = name;
      texture = TextureMap(name);
      isTextured = true;
    }
		uint32_t getDiffuseColour() {
			return packedDiffuseRGB;
		}
    uint32_t getTexturePointColour(vec2 uAndV) {
			// cout << uAndV.x * texture.height << " " << uAndV.y * texture.width << "\n";
      return texture.pixels[int(uAndV.x * texture.height + 0.5) * texture.width + int(uAndV.y * texture.width + 0.5)];
			// upscaledFrameBuffer[i][j] = cobbles.pixels[i * cobbles.width + j];
    }
	string materialName;
	private:
		vec3 floatDiffuseColour;
		uint32_t packedDiffuseRGB = 0;
    std::string map_Kd;
    TextureMap texture;
};