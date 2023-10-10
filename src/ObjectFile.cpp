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
	ObjectFile(const char *filename, float scale) {
		file = filename;
		scaleFactor = scale;
		ifstream inputStream;
		inputStream.open(filename, std::ios::in);
		string line;
		for (int lines = 0; std::getline(inputStream,line); lines++) {
			string code = split(line, ' ').at(0);
			if (code.compare("mtllib") == 0) {
				materialLib = split(line, ' ').at(1);
				matLib = MaterialLib(materialLib);
				
			} else if (code.compare("o") == 0) {
				objects.push_back(Object(split(line, ' ').at(1)));
			} else if (code.compare("usemtl") == 0) {
				objects.at(objects.size() - 1).setMaterial(split(line, ' ').at(1));
			} else if (code.compare("v") == 0) {
				vec3 vertex = parseVertex(line);
				vertices.push_back(vertex * scaleFactor);
			} else if (code.compare("f") == 0) {
				vec3 face = parseFace(line);
				std::array<vec3, 3> faceVertices;
				faceVertices[0] = vertices.at(face.x - 1);
				faceVertices[1] = vertices.at(face.y - 1);
				faceVertices[2] = vertices.at(face.z - 1);
				ModelTriangle faceTriangle; 
				faceTriangle.vertices = faceVertices;
				objects.at(objects.size() - 1).triangles.push_back(faceTriangle);
				faces.push_back(face);
			}
		}
		inputStream.close();
	}
	void printObjectMaterials() {
		for (Object object : objects) {
			Material mat = matLib.getMaterials()[object.material];
			uint32_t col = mat.getDiffuseColour();
			cout << object.name << " is " << mat.materialName << " which is " << col << '\n';
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
	void printFaces() {
		cout << "Faces of " << file << std::endl;
		for (int i = 0; i < faces.size(); i++) {
			vec3 current = faces.at(i);
			cout << '(' << current.x << ", " << current.y << ", " << current.z << ")" << std::endl;
		}
		cout << std::endl;
		cout << "Test triangle: ";
		cout << objects.at(0).triangles.at(0);
	}

	vector<Object> getObjects() {
		return objects;
	}

	Colour getKdOf(Object object) {
		uint32_t ci = matLib.getMaterials()[object.material].getDiffuseColour();
		return Colour((ci & 0x00FF0000) >> 16, (ci & 0x0000FF00) >> 8, ci & 0x000000FF);
	}

	private:

  static vec3 parseVertex(std::string input) {
    vector<std::string> splitStr = split(input, ' ');
    vec3 result(stof(splitStr.at(1)), stof(splitStr.at(2)), stof(splitStr.at(3)));
    return result;
  }

  static vec3 parseFace(std::string input) {
    vector<std::string> splitStr = split(input, ' ');
    int a, b, c;
    a = atoi(split(splitStr.at(1), '/').at(0).c_str());
    b = atoi(split(splitStr.at(2), '/').at(0).c_str());
    c = atoi(split(splitStr.at(3), '/').at(0).c_str());
    return vec3(a, b, c);
  }
	MaterialLib matLib;
	string materialLib;
	vector<vec3> vertices;
	vector<vec3> faces;
	vector<Object> objects;
	const char *file;
	float scaleFactor;
};

