#pragma once

#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <TextureMap.h>
#include <ModelTriangle.h>
#include <iostream>
#include <unordered_map>
#include "Object.cpp"
#include "MaterialLib.cpp"

using std::vector;
using glm::vec3;
using glm::vec2;
using std::string;
using std::ifstream;
using std::stof;
using std::atoi;
using std::cout;

class ObjectFile {
	public:
  MaterialLib *matLib;
	ObjectFile(std::string filename, float scale) {
		file = "assets/obj/" + filename;
		scaleFactor = scale;
		ifstream inputStream;
		inputStream.open(file, std::ios::in);
		string line;
    // Default object, material library and material in case .obj does not define them
    objects.push_back(Object("default"));
    objects.back().setMaterial("default");
    matLib = new MaterialLib();
    uint32_t smoothingGroup = 0;
		while (std::getline(inputStream,line)) {
			string code = split(line, ' ').at(0);
			if (code.compare("mtllib") == 0) {
				materialLib = split(line, ' ').at(1);
				matLib = new MaterialLib(materialLib);
			} else if (code.compare("o") == 0) {
				objects.push_back(Object(split(line, ' ').at(1)));
			} else if (code.compare("usemtl") == 0) {
				objects.back().setMaterial(split(line, ' ').at(1));
			} else if (code.compare("v") == 0) {
				vec3 vertex = parseVertex(line);
				vertices.push_back(vertex * scaleFactor);
      } else if (code.compare("s") == 0) {
        string option = split(line, ' ').at(1);
        if (code.compare("off") == 0) smoothingGroup = 0;
        else smoothingGroup = atoi(option.c_str());
      } else if (code.compare("vn") == 0) {
        vec3 vertexNormal = parseVertex(line);
        vertexNormals.push_back(vertexNormal);
      } else if (code.compare("vt") == 0) {
        vec2 vertexTextureRatio = parseTextureRatio(line);
        vertexTextureRatios.push_back(vertexTextureRatio);
      } else if (code.compare("f") == 0) {
				std::array<std::array<int, 3>, 3> face = parseFace(line);
				std::array<vec4, 3> faceVertices;
				faceVertices[0] = vec4(vertices.at(face[0][0] - 1), 1);
				faceVertices[1] = vec4(vertices.at(face[1][0] - 1), 1);
				faceVertices[2] = vec4(vertices.at(face[2][0] - 1), 1);
				Colour colour = getKdOf(objects.back());
        Material *material = &(matLib->materials.at(objects.back().material));
        std::array<vec2, 3> ts = {vec2(0), vec2(0), vec2(0)};
        if (face[0][1] > 0){
          ts = std::array<vec2, 3>{
            vertexTextureRatios.at(face[0][1] - 1),
            vertexTextureRatios.at(face[1][1] - 1),
            vertexTextureRatios.at(face[2][1] - 1),
            };
        }
        std::array<vec3, 3> vNs = {vec3(0), vec3(0), vec3(0)};
        bool hasVNs = false;
        if (face[0][2] > 0){
          vNs = std::array<vec3, 3>{
            vertexNormals.at(face[0][2] - 1),
            vertexNormals.at(face[1][2] - 1),
            vertexNormals.at(face[2][2] - 1),
            };
          hasVNs = true;
        }
        uint32_t sG = smoothingGroup;
        vec3 e0 = glm::normalize(vec3(faceVertices[0] - faceVertices[1]));
        vec3 e1 = glm::normalize(vec3(faceVertices[0] - faceVertices[2]));
        vec3 normal = glm::normalize(glm::cross(e0, e1));
				objects.back().triangles.push_back(ModelTriangle(faceVertices, ts, colour, normal, material, sG, vNs, hasVNs));
				faces.push_back(face);
			}
		}
		inputStream.close();
	}

	void printVertices() {
		cout << "Vertices of " << file << std::endl;
		for (int i = 0; i < vertices.size(); i++) {
			vec3 current = vertices.at(i);
			cout << '(' << current.x << ", " << current.y << ", " << current.z << ")" << std::endl;
		}
		cout << std::endl;
	}

	vector<Object> getObjects() {
		return objects;
	}

	Colour getKdOf(Object object) {
		uint32_t ci = matLib->materials.at(object.material).getDiffuseColourInt();
		return Colour((ci & 0x00FF0000) >> 16, (ci & 0x0000FF00) >> 8, ci & 0x000000FF);
	}

  void translate(vec4 displacement) {
    for (int i = 0; i < objects.size(); i++) {
      objects[i].translate(displacement);
    }
  }

  void centerOn(vec4 target) {
    float maxX = 100000.0f; float maxY = 100000.0f; float maxZ = 100000.0f;
    float minX = -100000.0f; float minY = -100000.0f; float minZ = -100000.0f;
    for (Object object : objects) {
      for (ModelTriangle triangle : object.triangles) {
        for (vec4 vertex : triangle.vertices) {
          maxX = glm::min(vertex.x, maxX);
          maxY = glm::min(vertex.y, maxY);
          maxZ = glm::min(vertex.z, maxZ);
          minX = glm::max(vertex.x, minX);
          minY = glm::max(vertex.y, minY);
          minZ = glm::max(vertex.z, minZ);
        }
      }
    }
    vec4 center = vec4((maxX + minX) / 2, (maxY + minY) / 2, (maxZ + minZ) / 2, 1);
    translate(target - center);
  }

	private:

  static vec3 parseVertex(std::string input) {
    vector<std::string> splitStr = split(input, ' ');
    vec3 result(stof(splitStr.at(1)), stof(splitStr.at(2)), stof(splitStr.at(3)));
    return result;
  }

  static vec2 parseTextureRatio(std::string input) {
    vector<std::string> splitStr = split(input, ' ');
    vec2 result(stof(splitStr.at(1)), stof(splitStr.at(2)));
    return result;
  }

  static std::array<std::array<int, 3>, 3> parseFace(std::string input) {
    // vertex/texture/normal format
    std::array<std::array<int, 3>, 3> faceData;
    vector<std::string> splitStr = split(input, ' ');
    for (int i = 0; i < 3; i++) {
      vector<string> information = split(splitStr.at(i + 1), '/');
      string locationIndex = information.at(0);
      string normalIndex = "-1";
      string textureIndex = "-1";
      if (information.size() > 1) {
        if (!information.at(1).empty()) {
          textureIndex = information.at(1);
        }
      }
      if (information.size() > 2) {
        if (!information.at(2).empty()) {
          normalIndex = information.at(2);
        }
      }
      faceData[i] = std::array<int, 3>{atoi(locationIndex.c_str()), atoi(textureIndex.c_str()), atoi(normalIndex.c_str())};
    }
    return faceData;
  }
	string materialLib;
	vector<vec3> vertices;
  vector<vec3> vertexNormals;
  vector<vec2> vertexTextureRatios;
	vector<std::array<std::array<int, 3>, 3>> faces;
	vector<Object> objects;
	std::string file;
	float scaleFactor;
};

