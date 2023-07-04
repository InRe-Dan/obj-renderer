#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

#define WIDTH 320
#define HEIGHT 240

using std::vector;
using glm::vec3;

vector<float> interpolateSingleFloats(float from, float to, int steps) {
	float diff = to - from;
	float interval = diff / steps;
	vector<float> interpolation;
	for (int i = 0; i < steps; i++) {
		interpolation.push_back(from + i * interval);
	}
	return interpolation;
}

vector<vec3> interpolateThreeElementValues(vec3 from, vec3 to, int steps) {
	vec3 diff = to - from;
	vec3 interval = diff * float(1 / steps);
	vector<vec3> interpolation;
	for (int i = 0; i < steps; i++) {
		interpolation.push_back(from + float(i) * interval);
	}
	return interpolation;
}

uint32_t vec3ToColour(vec3 vect, int alpha) {
	uint32_t colour = (alpha << 24) + (int(vect.x) << 16) + (int(vect.y) << 8) + int(vect.z);
	return colour; 
}

 
void draw(DrawingWindow &window) {
	window.clearPixels();

	vec3 topLeft(255, 0, 0);        // red 
	std::cout << std::bitset<32>(vec3ToColour(vec3(128, 64, 32), 255)) << std::endl;
	vec3 topRight(0, 0, 255);       // blue 
	vec3 bottomRight(0, 255, 0);    // green 
	vec3 bottomLeft(255, 255, 0);   // yellow

	vector<vec3> leftEdge = interpolateThreeElementValues(topLeft, bottomLeft, HEIGHT);
	vector<vec3> rightEdge = interpolateThreeElementValues(topRight, bottomRight, HEIGHT);

	for (size_t y = 0; y < window.height; y++) {
		window.setPixelColour(0, y,vec3ToColour(leftEdge.at(y), 255));
		window.setPixelColour(0, window.height-1,vec3ToColour(rightEdge.at(y), 255));
		vector<vec3> layer = interpolateThreeElementValues(leftEdge.at(y), rightEdge.at(y), window.width);
		for (size_t x = 0; x < window.width; x++) {
			window.setPixelColour(x, y, vec3ToColour(layer.at(x), 255));
		}
	}

}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		draw(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
