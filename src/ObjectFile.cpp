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
using glm::round;
using std::string;
using std::ifstream;
using std::stof;
using std::atoi;
using std::cout;

class ObjectFile {
	public:
  MaterialLib *matLib;
	ObjectFile(const char *filename, float scale) {
		file = filename;
		scaleFactor = scale;
		ifstream inputStream;
		inputStream.open(filename, std::ios::in);
		string line;
		while (std::getline(inputStream,line)) {
			string code = split(line, ' ').at(0);
			if (code.compare("mtllib") == 0) {
				materialLib = split(line, ' ').at(1);
        cout << "passsing" << materialLib << "\n";
				matLib = new MaterialLib(materialLib);
			} else if (code.compare("o") == 0) {
				objects.push_back(Object(split(line, ' ').at(1)));
			} else if (code.compare("usemtl") == 0) {
				objects.at(objects.size() - 1).setMaterial(split(line, ' ').at(1));
			} else if (code.compare("v") == 0) {
				vec3 vertex = parseVertex(line);
				vertices.push_back(vertex * scaleFactor);
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
				ModelTriangle faceTriangle; 
				faceTriangle.vertices = faceVertices;
				faceTriangle.colour = getKdOf(objects.at(objects.size() - 1));
        if (face[0][1] > 0){
          faceTriangle.texturePoints = std::array<TexturePoint, 3>{
            TexturePoint(vertexTextureRatios.at(face[0][1] - 1).x, vertexTextureRatios.at(face[0][1] - 1).y),
            TexturePoint(vertexTextureRatios.at(face[1][1] - 1).x, vertexTextureRatios.at(face[1][1] - 1).y),
            TexturePoint(vertexTextureRatios.at(face[2][1] - 1).x, vertexTextureRatios.at(face[2][1] - 1).y)
            };
        }
        vec3 e0 = vec3(faceTriangle.vertices[0] - faceTriangle.vertices[1]);
        vec3 e1 = vec3(faceTriangle.vertices[0] - faceTriangle.vertices[2]);
        faceTriangle.normal = glm::normalize(glm::cross(e0, e1));
				objects.at(objects.size() - 1).triangles.push_back(faceTriangle);
				faces.push_back(face);
			}
		}
		inputStream.close();
	}
	void printObjectMaterials() {
		for (Object object : objects) {
			Material mat = matLib->getMaterials()[object.material];
			uint32_t col = mat.getDiffuseColour();
			cout << object.name << " is " << mat.materialName << " which is " << col << " and isTextured is " << mat.isTextured << "\n";
			cout << "RGB: " << ((col & 0x00FF0000) >> 16) << " " << ((col & 0x0000FF00) >> 8) << " " << (col & 0x000000FF) << '\n';
			cout << "Object has " << object.triangles.size() << " triangles.\n";
		}
		cout << std::endl;
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
		uint32_t ci = matLib->getMaterials()[object.material].getDiffuseColour();
		return Colour((ci & 0x00FF0000) >> 16, (ci & 0x0000FF00) >> 8, ci & 0x000000FF);
	}

  void translate(vec4 displacement) {
    for (int i = 0; i < objects.size(); i++) {
      objects[i].translate(displacement);
    }
  }

  void centerOn(vec4 target) {
    vec4 sum = vec4(0);
    float count = 0;
    for (Object object : objects) {
      for (ModelTriangle triangle : object.triangles) {
        count += 3;
        sum += triangle.vertices[0] + triangle.vertices[1] + triangle.vertices[2];
      }
    }
    vec4 average = sum / count;
    translate(target - average);
  }

	private:

  static vec3 parseVertex(std::string input) {
    vector<std::string> splitStr = split(input, ' ');
    // TODO what were you thinking with this negative sign? you cost us so much debugging...
    vec3 result(- stof(splitStr.at(1)), stof(splitStr.at(2)), stof(splitStr.at(3)));
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
	const char *file;
	float scaleFactor;
};

