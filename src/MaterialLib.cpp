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
#include "Material.cpp"

using std::vector;
using glm::vec3;
using glm::vec2;
using std::string;
using std::ifstream;
using std::stof;
using std::atoi;
using std::cout;

class MaterialLib {
	public:
		MaterialLib(string filename) {
			file = filename;
			ifstream inputStream;
			inputStream.open(filename, std::ios::in);
			string line, code, materialName;
      cout << "reading material: ";
      cout << filename << "\n";
			while (std::getline(inputStream,line)) {
				string code = split(line, ' ').at(0);
        cout << code << "\n";
				if (code.compare("newmtl") == 0) {
					materialName = split(line, ' ').at(1);
					Material newMatObject(materialName);
					materials[materialName] = newMatObject;
				} else if (code.compare("Kd") == 0) {
					vector<string> lineSplit = split(line, ' ');
					vec3 rgb(stof(lineSplit.at(1)), stof(lineSplit.at(2)), stof(lineSplit.at(3)));
					materials[materialName].setDiffuseColour(rgb);
				} else if (code.compare("map_Kd") == 0) {
          vector<string> lineSplit = split(line, ' ');
          cout << "yup!!\n";
          materials[materialName].setMap_Kd(lineSplit.at(1));
        }
			}
			inputStream.close();
			printMaterials();
		}
		std::unordered_map<string, Material> getMaterials() {
			return materials;
		}
		void printMaterials() {
			std::unordered_map<string, Material>::iterator itr;
			for(itr=materials.begin();itr!=materials.end();itr++)
			{
				cout<<itr->first<<" "<<itr->second.getDiffuseColour()<< " " << itr->second.isTextured<< std::endl;
			}
		}
	private:
		string file;
		std::unordered_map<string, Material> materials;
};