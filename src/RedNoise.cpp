#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

#define WIDTH 320
#define HEIGHT 240
#define WHITE Colour(255, 255, 255)
#define RED Colour(255, 0, 0)
#define PURPLE Colour(255, 0, 255)

using std::vector;
using glm::vec3;

vector<CanvasTriangle> triangleVector;
vector<Colour> colourVector;

uint32_t vec3ToColour(vec3 vect, int alpha) {
	uint32_t colour = (alpha << 24) + (int(vect.x) << 16) + (int(vect.y) << 8) + int(vect.z);
	return colour; 
}

void line(CanvasPoint to, CanvasPoint from, Colour colour, DrawingWindow &window) {
	// TODO use existing functions? workbook implies so
	float xDiff = to.x - from.x;
	float yDiff = to.y - from.y;
	float steps = glm::max(glm::abs(xDiff), glm::abs(yDiff));
	float xStepSize = xDiff / steps;
	float yStepSize = yDiff / steps;
	vec3 colVect(colour.red, colour.green, colour.blue);
	for (float i = 0.0; i < steps; i++) {
		float x = from.x + (xStepSize * i);
		float y = from.y + (yStepSize * i);
		window.setPixelColour(x, y, vec3ToColour(colVect, 255));
	}
	
}

void strokedTriangle(CanvasTriangle &triangle, Colour colour, DrawingWindow &window) {
	line(triangle.v0(), triangle.v1(), colour, window);
	line(triangle.v1(), triangle.v2(), colour, window);
	line(triangle.v2(), triangle.v0(), colour, window);
}

void addStrokedTriangle() {
	CanvasPoint v0(rand() % (WIDTH - 1), rand() % (HEIGHT - 1));
	CanvasPoint v1(rand() % (WIDTH - 1), rand() % (HEIGHT - 1));
	CanvasPoint v2(rand() % (WIDTH - 1), rand() % (HEIGHT - 1));
	triangleVector.push_back(CanvasTriangle(v0, v1, v2));
	colourVector.push_back(Colour(rand() % 256, rand() % 256, rand() % 256));
}

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
	vec3 interval = diff * float(1) / float(steps);
	vector<vec3> interpolation;
	for (int i = 0; i < steps; i++) {
		interpolation.push_back(from + float(i) * interval);
	}
	return interpolation;
}

 
void draw(DrawingWindow &window) {
	static int ticks;
	ticks++;

	vec3 topLeft(255, 0, 0);        // red 
	vec3 topRight(0, 0, 255);       // blue 
	vec3 bottomRight(0, 255, 0);    // green 
	vec3 bottomLeft(255, 255, 0);   // yellow

	vector<vec3> leftEdge = interpolateThreeElementValues(topLeft, bottomLeft, HEIGHT);
	vector<vec3> rightEdge = interpolateThreeElementValues(topRight, bottomRight, HEIGHT);

	for (size_t y = 0; y < window.height; y++) {
		vector<vec3> layer = interpolateThreeElementValues(leftEdge.at(y), rightEdge.at(y), window.width);
		for (size_t x = 0; x < window.width; x++) {
			window.setPixelColour(x, y, vec3ToColour(layer.at(x), 255));
		}
	}
	for (int i = 0; i < triangleVector.size(); i++) {
		strokedTriangle(triangleVector.at(i), colourVector.at(i), window);
	}

}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_u) addStrokedTriangle();
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

void test() {
	vec3 one(255, 0, 0);
	vec3 two(0, 255, 0);
	vector<vec3> thing = interpolateThreeElementValues(one, two, 3);
	std::cout.flush();
	quick_exit(0);
}


int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	// test();
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		draw(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
