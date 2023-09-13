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
#include "objutil.cpp"
#include "vecutil.cpp"

#define WIDTH 640
#define HEIGHT 480
#define WHITE Colour(255, 255, 255)
#define RED Colour(255, 0, 0)
#define PURPLE Colour(255, 0, 255)

using std::vector;
using glm::vec3;
using glm::vec2;
using glm::round;

vector<CanvasTriangle> triangleVector;
vector<Colour> colourVector;
vector<CanvasTriangle> filledTriangleVector;
vector<Colour> filledColourVector;

// Convention will be that the positive Z axis goes from the image plane towards the camera.
vec3 cameraPosition(0.0, 0.0, 4);
float focalLength = 2;
ObjectFile cornell("cornell-box.obj", 0.17f);

TextureMap brickMap("texture.ppm");

CanvasPoint getCanvasIntersectionPoint(vec3 cameraPosition, vec3 vertexPosition, float focalLength) {
	// CanvasPoint = u, v
	// Vertex = x, y, z
	// Focal length = f
	// Plane dimensions = W, H
	// u = f * (x/z) + W/2
	// v = f * (y/z) + H/2
	// All coordinates are relative to the camera!

	// First we need to translate the vertex point
	vec3 vertexToCamera = vertexPosition - cameraPosition;

	float u = focalLength * (vertexToCamera.x/vertexToCamera.z) * 50 + WIDTH/2;
	float v = focalLength * (vertexToCamera.y/vertexToCamera.z) * 50+ HEIGHT/2;
	return CanvasPoint(u, v);
}

void line(CanvasPoint to, CanvasPoint from, Colour colour, DrawingWindow &window) {
	// TODO use existing functions? workbook implies so
	float xDiff = to.x - from.x;
	float yDiff = to.y - from.y;
	float steps = glm::max(glm::abs(xDiff), glm::abs(yDiff));
	float xStepSize = xDiff / steps;
	float yStepSize = yDiff / steps;
	vec3 colVect(colour.red, colour.green, colour.blue);
	float x = from.x;
	float y = from.y;
	for (int i = 0; i < steps; i++) {
		x += xStepSize;
		y += yStepSize;
		window.setPixelColour(round(x), round(y), vec3ToColour(colVect, 255));
	}
	
}

void strokedTriangle(CanvasTriangle &triangle, Colour colour, DrawingWindow &window) {
	line(triangle.v0(), triangle.v1(), colour, window);
	line(triangle.v1(), triangle.v2(), colour, window);
	line(triangle.v2(), triangle.v0(), colour, window);
}

void filledTriangle(CanvasTriangle &triangle, Colour colour, DrawingWindow &window) {
	// Sort vertices by height
	CanvasPoint top = triangle.v0();
	CanvasPoint mid = triangle.v1();
	CanvasPoint bot = triangle.v2();
	if (top.y > mid.y) std::swap(top, mid);
	if (mid.y > bot.y) std::swap(mid, bot);
	if (top.y > mid.y) std::swap(top, mid);

	// Locate point at the same y level from middle vertex
	const float height = glm::abs(top.y - bot.y);
	vector<vec2> topToBot = interpolate(top, bot, height);
	int imaginaryY = 0;
	int imaginaryX = 0;
	for (int i = 0; i < height; i++) {
		if (topToBot.at(i).y == mid.y) {
			imaginaryY = i + top.y;
			imaginaryX = topToBot.at(i).x;
			break;
		}
	}
	CanvasPoint imaginary(imaginaryX, imaginaryY);
	const float topHeight = glm::abs(top.y - mid.y);
	const float botHeight = glm::abs(bot.y - mid.y);

	// Interpolate lines between all 4 points
	vector<vec2> topToImaginary = interpolate(top, imaginary, topHeight);
	vector<vec2> topToMid = interpolate(top, mid, topHeight);
	vector<vec2> imaginaryToBot = interpolate(imaginary, bot, botHeight);
	vector<vec2> midToBot = interpolate(mid, bot, botHeight);
	// Colour between top two lines
	for (int i = 0; i < topHeight; i++) {
		vec2 to = topToImaginary.at(i);
		vec2 from = topToMid.at(i);
		line(CanvasPoint(to.x, to.y), CanvasPoint(from.x, from.y), colour, window);
	}
	// Colour between bottom two lines
	for (int i = 0; i < botHeight; i++) {
		vec2 to = imaginaryToBot.at(i);
		vec2 from = midToBot.at(i);
		line(CanvasPoint(to.x, to.y), CanvasPoint(from.x, from.y), colour, window);
	}
	// Outline the triangle (For debug purposes)
	strokedTriangle(triangle, WHITE, window);
}

void texturedTriangle(CanvasTriangle &triangle, TextureMap &map, DrawingWindow &window) {
	// Sort vertices by height
	CanvasPoint top = triangle.v0();
	CanvasPoint mid = triangle.v1();
	CanvasPoint bot = triangle.v2();
	if (top.y > mid.y) std::swap(top, mid);
	if (mid.y > bot.y) std::swap(mid, bot);
	if (top.y > mid.y) std::swap(top, mid);

	// Locate point at the same y level from middle vertex
	const float height = glm::abs(top.y - bot.y);
	vector<vec2> topToBot = interpolate(top, bot, height);
	vector<vec2> topToBotT = interpolate(top.texturePoint, bot.texturePoint, height);
	int imaginaryY = 0;
	int imaginaryX = 0;
	int imaginaryYT = 0;
	int imaginaryXT = 0;
	for (int i = 0; i < height; i++) {
		if (topToBot.at(i).y == mid.y) {
			imaginaryY = topToBot.at(i).y;
			imaginaryX = topToBot.at(i).x;
			imaginaryYT = topToBotT.at(i).y;
			imaginaryXT = topToBotT.at(i).x;
			break;
		}
	}
	CanvasPoint imaginary(imaginaryX, imaginaryY);
	imaginary.texturePoint.x = imaginaryXT;
	imaginary.texturePoint.y = imaginaryYT;
	const float topHeight = glm::abs(top.y - mid.y);
	const float botHeight = glm::abs(bot.y - mid.y);

	// Interpolate lines between all 4 points
	vector<vec2> topToImaginary = interpolate(top, imaginary, topHeight);
	vector<vec2> topToMid = interpolate(top, mid, topHeight);
	vector<vec2> imaginaryToBot = interpolate(imaginary, bot, botHeight);
	vector<vec2> midToBot = interpolate(mid, bot, botHeight);

	vector<vec2> topToImaginaryT = interpolate(top.texturePoint, imaginary.texturePoint, topHeight);
	vector<vec2> topToMidT = interpolate(top.texturePoint, mid.texturePoint, topHeight);
	vector<vec2> imaginaryToBotT = interpolate(imaginary.texturePoint, bot.texturePoint, botHeight);
	vector<vec2> midToBotT = interpolate(mid.texturePoint, bot.texturePoint, botHeight);
	// Colour between top two lines
	for (int i = 0; i < topHeight; i++) {
		vec2 to = topToImaginary.at(i);
		vec2 from = topToMid.at(i);
		vec2 toT = topToImaginaryT.at(i);
		vec2 fromT = topToMidT.at(i);
		int width = abs(to.x - from.x);
		vector<vec2> interpolated = interpolate(to, from, width);
		vector<vec2> interpolatedT = interpolate(toT, fromT, width);
		for (int j = 0; j < width; j++) {
			int x = round(interpolated.at(j).x);
			int y = round(interpolated.at(j).y);
			int xT = round(interpolatedT.at(j).x);
			int yT = round(interpolatedT.at(j).y);
			window.setPixelColour(x, y, map.pixels.at(yT * map.width + xT));
		}
	}
	// Colour between bottom two lines
	for (int i = 0; i < botHeight; i++) {
		vec2 to = imaginaryToBot.at(i);
		vec2 from = midToBot.at(i);
		vec2 toT = imaginaryToBotT.at(i);
		vec2 fromT = midToBotT.at(i);
		int width = abs(to.x - from.x);
		vector<vec2> interpolated = interpolate(to, from, width);
		vector<vec2> interpolatedT = interpolate(toT, fromT, width);
		for (int j = 0; j < width; j++) {
			int x = round(interpolated.at(j).x);
			int y = round(interpolated.at(j).y);
			int xT = round(interpolatedT.at(j).x);
			int yT = round(interpolatedT.at(j).y);
			window.setPixelColour(x, y, map.pixels.at(yT * map.width + xT));
		}
	}
	strokedTriangle(triangle, WHITE, window);
}

void addStrokedTriangle() {
	CanvasPoint v0(rand() % (WIDTH - 1), rand() % (HEIGHT - 1));
	CanvasPoint v1(rand() % (WIDTH - 1), rand() % (HEIGHT - 1));
	CanvasPoint v2(rand() % (WIDTH - 1), rand() % (HEIGHT - 1));
	triangleVector.push_back(CanvasTriangle(v0, v1, v2));
	colourVector.push_back(Colour(rand() % 256, rand() % 256, rand() % 256));
}
void addFilledTriangle() {
	CanvasPoint v0(rand() % (WIDTH - 1), rand() % (HEIGHT - 1));
	CanvasPoint v1(rand() % (WIDTH - 1), rand() % (HEIGHT - 1));
	CanvasPoint v2(rand() % (WIDTH - 1), rand() % (HEIGHT - 1));
	filledTriangleVector.push_back(CanvasTriangle(v0, v1, v2));
	filledColourVector.push_back(Colour(rand() % 256, rand() % 256, rand() % 256));
}

 
void draw(DrawingWindow &window) {
	static int ticks;
	ticks++;

	vec3 topLeft(255, 0, 0);        // red 
	vec3 topRight(0, 0, 255);       // blue 
	vec3 bottomRight(0, 255, 0);    // green 
	vec3 bottomLeft(255, 255, 0);   // yellow

	vector<vec3> leftEdge = interpolate(topLeft, bottomLeft, HEIGHT);
	vector<vec3> rightEdge = interpolate(topRight, bottomRight, HEIGHT);

	for (size_t y = 0; y < window.height; y++) {
		vector<vec3> layer = interpolate(leftEdge.at(y), rightEdge.at(y), window.width);
		for (size_t x = 0; x < window.width; x++) {
			window.setPixelColour(x, y, 0xff000000);
		}
	}
	for (int i = 0; i < triangleVector.size(); i++) {
		filledTriangle(triangleVector.at(i), colourVector.at(i), window);
	}
	for (int i = 0; i < filledTriangleVector.size(); i++) {
		filledTriangle(filledTriangleVector.at(i), filledColourVector.at(i), window);
	}
	vector<ModelTriangle> cornellTriangles = cornell.getTriangles();
	for (int i = 0; i < cornellTriangles.size(); i++) {
		CanvasPoint a = getCanvasIntersectionPoint(cameraPosition, cornellTriangles.at(i).vertices.at(0), focalLength);
		CanvasPoint b = getCanvasIntersectionPoint(cameraPosition, cornellTriangles.at(i).vertices.at(1), focalLength);
		CanvasPoint c = getCanvasIntersectionPoint(cameraPosition, cornellTriangles.at(i).vertices.at(2), focalLength);
		CanvasTriangle canvasTriangle(a, b, c);
		filledTriangle(canvasTriangle, cornellTriangles.at(i).colour, window);
		strokedTriangle(canvasTriangle, WHITE, window);
	}
	/*
	CanvasPoint _one(160, 10); _one.texturePoint.x = 195, _one.texturePoint.y = 5;
	CanvasPoint _two(300, 230); _two.texturePoint.x = 395; _two.texturePoint.y = 380;
	CanvasPoint _three(10, 150); _three.texturePoint.x = 65; _three.texturePoint.y = 330;
	const CanvasPoint one = _one;
	const CanvasPoint two = _two;
	// const CanvasPoint three = _three;
	// CanvasTriangle triangle(one, two, three);
	// texturedTriangle(triangle, brickMap, window);
	*/

}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) cameraPosition += vec3(-0.1, 0, 0);
		else if (event.key.keysym.sym == SDLK_RIGHT) cameraPosition += vec3(+0.1, 0, 0);
		else if (event.key.keysym.sym == SDLK_UP) cameraPosition += vec3(0, 0.1, 0);
		else if (event.key.keysym.sym == SDLK_DOWN) cameraPosition += vec3(0, -0.1, 0);
		else if (event.key.keysym.sym == SDLK_w) cameraPosition += vec3(0, 0, -0.1);
		else if (event.key.keysym.sym == SDLK_s) cameraPosition += vec3(0, 0, +0.1);
		else if (event.key.keysym.sym == SDLK_a) focalLength -= 0.1;
		else if (event.key.keysym.sym == SDLK_d) focalLength += 0.1;
		else if (event.key.keysym.sym == SDLK_u) addStrokedTriangle();
		else if (event.key.keysym.sym == SDLK_f) addFilledTriangle();
		cout << "Camera Position: " << cameraPosition.x << " " << cameraPosition.y << " " << cameraPosition.z << std::endl;
		cout << "Focal Length: " << focalLength << std::endl;

	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

void test() {
	vec3 one(255, 0, 0);
	vec3 two(0, 255, 0);
	vector<vec3> thing = interpolate(one, two, 3);
	std::cout.flush();
	quick_exit(0);
}


int main(int argc, char *argv[]) {
	// srand(time(NULL));
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
