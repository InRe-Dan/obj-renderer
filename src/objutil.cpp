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

using std::vector;
using glm::vec3;
using glm::vec2;
using glm::round;

vec3 parseVector(std::string input) {

}

vec3 parseFace(std::string input) {

}

vector<ModelTriangle> readOBJ(const char *filename, float scale) {
	std::ifstream inputStream;
	inputStream.open(filename, std::ios::in);
	vector<vec3> vertices;
	vector<ModelTriangle> triangles;
	std::string line;
	for (int lines = 0; std::getline(inputStream,line); lines++) {
        vector<std::string> splitLine = split(line, ' ');

        if (splitLine.at(0).compare("v") == 0) {

        }
        // And so on
	}
	inputStream.close();
	return triangles;
}
