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

using std::vector;
using glm::vec3;
using glm::vec2;
using glm::round;
using std::string;
using std::ifstream;
using std::stof;
using std::atoi;
using std::cout;


vec3 parseVertex(std::string input) {
	vector<std::string> splitStr = split(input, ' ');
	vec3 result(stof(splitStr.at(1)), stof(splitStr.at(2)), stof(splitStr.at(3)));
	return result;
}

vec3 parseFace(std::string input) {
	vector<std::string> splitStr = split(input, ' ');
	int a, b, c;
	a = atoi(split(splitStr.at(1), '/').at(0).c_str());
	b = atoi(split(splitStr.at(2), '/').at(0).c_str());
	c = atoi(split(splitStr.at(3), '/').at(0).c_str());
	return vec3(a, b, c);
}

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
			packedDiffuseRGB += int(colour.z);
		}
		uint32_t getDiffuseColour() {
			return packedDiffuseRGB;
		}
	private:
		string materialName;
		vec3 floatDiffuseColour;
		uint32_t packedDiffuseRGB = 0;
};

class MaterialLibrary {
	public:
		MaterialLibrary(string filename="UNSET") {
			file = filename;
			ifstream inputStream;
			inputStream.open(filename, std::ios::in);
			string line, code, materialName;
			for (int lines = 0; std::getline(inputStream,line); lines++) {
				string code = split(line, ' ').at(0);
				if (code.compare("newmtl") == 0) {
					materialName = split(line, ' ').at(1);
					Material newMatObject(materialName);
					materials[materialName] = newMatObject;
				} else if (code.compare("Kd") == 0) {
					vector<string> lineSplit = split(line, ' ');
					vec3 rgb(stof(lineSplit.at(1)), stof(lineSplit.at(2)), stof(lineSplit.at(3)));
					materials[materialName].setDiffuseColour(rgb);
				}
			}
			inputStream.close();
			printMaterials();
		}
		void printMaterials() {
			std::unordered_map<string, Material>::iterator itr;
			for(itr=materials.begin();itr!=materials.end();itr++)
			{
				cout<<itr->first<<" "<<itr->second.getDiffuseColour()<<std::endl;
			}
		}
	private:
		string file;
		std::unordered_map<string, Material> materials;
};

class Object {
	public:
	Object(string id) {
		name = id;
	}
	void setMaterial(string m) {
		material = m;
	}
	vector<ModelTriangle> triangles;
	private:
	string material;
	string name;
};

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
				matLib = MaterialLibrary(materialLib);
				
			} else if (code.compare("o") == 0) {
				objects.push_back(Object(split(line, ' ').at(1)));
			} else if (code.compare("usemtl") == 0) {
				objects.at(objects.size() - 1).setMaterial(split(line, ' ').at(1));
			} else if (code.compare("v") == 0) {
				vec3 vertex = parseVertex(line);
				vertices.push_back(vertex);
			} else if (code.compare("f") == 0) {
				vec3 face = parseFace(line);
				std::array<vec3, 3> faceVertices;
				faceVertices[0] = vertices.at(face.x - 1);
				faceVertices[1] = vertices.at(face.y - 1);
				faceVertices[2] = vertices.at(face.x - 1);
				ModelTriangle faceTriangle;
				faceTriangle.vertices = faceVertices;\
				objects.at(objects.size() - 1).triangles.push_back(faceTriangle);
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

	vector<ModelTriangle> getTriangles() {
		return vector<ModelTriangle>();
	}

	private:
	MaterialLibrary matLib;
	string materialLib;
	vector<vec3> vertices;
	vector<vec3> faces;
	vector<Object> objects;
	const char *file;
	float scaleFactor;
};

