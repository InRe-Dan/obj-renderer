#pragma once

#include <CanvasTriangle.h>
#include <vector>
#include <glm/glm.hpp>
#include <TextureMap.h>
#include <map>
#include "vecutil.cpp"

using std::vector;
using glm::vec3;
using glm::vec2;
using std::string;
using std::ifstream;
using std::stof;
using std::atoi;
using std::cout;

class Material {
	public:
    bool isTextured = false;
		bool hasNormalMap = false;
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
      texture = TextureMap("assets/texture/" + name);
      isTextured = true;
    }
		void setMap_Bump(string name) {
			map_bump = name;
			bump = TextureMap("assets/normal/" + name);
			hasNormalMap = true;
			bump_vectors = vector<vector<vec3>>();
			for (int i = 0; i < bump.height; i++) {
				bump_vectors.push_back(vector<vec3>());
				for (int j = 0; j < bump.width; j++) {
					uint32_t integer = texture.pixels[i * texture.width + j];
					vec3 vector = vec3((integer >> 16) & 0xFF, (integer >> 8) & 0xFF, (integer) & 0xFF);
					bump_vectors[i].push_back(glm::normalize(vector * 2.0f - vector));
				}
			}
		}
		uint32_t getDiffuseColour() {
			return packedDiffuseRGB;
		}
    uint32_t getTexturePointColour(vec2 uAndV) {
			// cout << uAndV.x * texture.height << " " << uAndV.y * texture.width << "\n";
      return texture.pixels[int(uAndV.x * texture.height + 0.5) * texture.width + int(uAndV.y * texture.width + 0.5)];
			// upscaledFrameBuffer[i][j] = cobbles.pixels[i * cobbles.width + j];
    }
		vec3 getNormalMapVector(vec2 uAndV) {
      return bump_vectors.at(roundI(uAndV.x * bump.height)).at(roundI(uAndV.y * bump.width));
			// upscaledFrameBuffer[i][j] = cobbles.pixels[i * cobbles.width + j];
    }
	string materialName;
	private:
		vec3 floatDiffuseColour;
		uint32_t packedDiffuseRGB = 0;
    std::string map_Kd;
		std::string map_bump;
    TextureMap texture;
		TextureMap bump;
		vector<vector<vec3>> bump_vectors;
};