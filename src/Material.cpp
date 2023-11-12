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
		Material() {
			materialName = "default";
		}
		Material(string name) {
			materialName = name;
		}
		void setDiffuseColour(vec3 colour) {
			diffuseDefined = true;
			floatDiffuseColour = colour;
			packedDiffuseRGB = 0;
			packedDiffuseRGB = 255 << 24;
			packedDiffuseRGB += int(colour.x * 255) << 16;
			packedDiffuseRGB += int(colour.y * 255) << 8;
			packedDiffuseRGB += int(colour.z * 255);
		}
		void setAmbientColour(vec3 colour) {
			ambientDefined = true;
			floatAmbientColour = colour;
			packedAmbientRGB = 0;
			packedAmbientRGB = 255 << 24;
			packedAmbientRGB += int(colour.x * 255) << 16;
			packedAmbientRGB += int(colour.y * 255) << 8;
			packedAmbientRGB += int(colour.z * 255);
		}
		void setSpecularColour(vec3 colour) {
			specularDefined = true;
			floatSpecularColour = colour;
			packedSpecularRGB = 0;
			packedSpecularRGB = 255 << 24;
			packedSpecularRGB += int(colour.x * 255) << 16;
			packedSpecularRGB += int(colour.y * 255) << 8;
			packedSpecularRGB += int(colour.z * 255);
		}
		void setSpecularExponent(float value) {
			specularExponent = value;
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
					uint32_t integer =  bump.pixels[i * bump.width + j];
					vec3 vector = vec3((integer >> 16) & 0xFF, (integer >> 8) & 0xFF, (integer) & 0xFF);
					vec3 adjustedVector = glm::normalize(((vector / 255.0f) -0.5f) * 2.0f);
					bump_vectors[i].push_back(adjustedVector);
				}
			}
		}
		vec3 getDiffuseColour() {
			return floatDiffuseColour;
		}
		uint32_t getDiffuseColourInt() {
			return packedDiffuseRGB;
		}
		vec3 getAmbientColour() {
			return floatAmbientColour;
		}
		vec3 getSpecularColour() {
			return floatSpecularColour;
		}
		float getSpecularExponent() {
			return specularExponent;
		}
    uint32_t getTexturePointColour(vec2 uAndV) {
			// cout << uAndV.x * texture.height << " " << uAndV.y * texture.width << "\n";
      return texture.pixels[roundI(uAndV.x * (texture.height-1)) * texture.width + roundI(uAndV.y * (texture.width-1))];
			// upscaledFrameBuffer[i][j] = cobbles.pixels[i * cobbles.width + j];
    }
		uint32_t getNormalMapRGB(vec2 uAndV) {
			// cout << uAndV.x * texture.height << " " << uAndV.y * texture.width << "\n";
      return texture.pixels[roundI(uAndV.x * (texture.height-1)) * texture.width + roundI(uAndV.y * (texture.width-1))];
			// upscaledFrameBuffer[i][j] = cobbles.pixels[i * cobbles.width + j];
    }
		vec3 getNormalMapVector(vec2 uAndV) {
      return bump_vectors.at(roundI((uAndV.x * (bump.height-1)))).at(roundI((uAndV.y * (bump.width-1))));
			// upscaledFrameBuffer[i][j] = cobbles.pixels[i * cobbles.width + j];
    }
		void finishLoading() {
			if (!diffuseDefined) {
				cout << "\nERROR!! A material was defined without a diffuse colour!" << std::endl;
				exit(1);
			} 
			if (!ambientDefined) {
			floatAmbientColour = floatDiffuseColour;
			packedAmbientRGB = packedDiffuseRGB;
			}
			if (!specularDefined) {
			floatSpecularColour = floatDiffuseColour;
			packedSpecularRGB = packedDiffuseRGB;
			}
		}
	string materialName;
	private:
		bool diffuseDefined = false;
		vec3 floatDiffuseColour = vec3(1);
		uint32_t packedDiffuseRGB = 0xFFFFFFFF;
		bool ambientDefined = false;
		vec3 floatAmbientColour = vec3(1);
		uint32_t packedAmbientRGB = 0xFFFFFFFF;
		float specularDefined = false;
		vec3 floatSpecularColour = vec3(0);
		uint32_t packedSpecularRGB = 0xFFFFFFFFF;
		float specularExponent = 0.0f;
    std::string map_Kd;
		std::string map_bump;
    TextureMap texture;
		TextureMap bump;
		vector<vector<vec3>> bump_vectors;
};