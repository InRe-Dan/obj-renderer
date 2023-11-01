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
#include "ObjectFile.cpp"
#include "Camera.cpp"
#include "font8x8_basic.h"
#include "postprocessing.cpp"
#include <chrono>
#include <ctime> 

#define WIDTH 600
#define HEIGHT 600
#define WHITE Colour(255, 255, 255)
#define RED Colour(255, 0, 0)
#define PURPLE Colour(255, 0, 255)

using std::vector;
using glm::vec3;
using glm::vec2;
using glm::round;


// Convention will be that the positive Z axis goes from the image plane towards the camera.
Camera camera(WIDTH, HEIGHT);
ObjectFile cornell("cornell-box.obj", 1.0f);
// Simple plane object (modified cornell) used for debugging
ObjectFile plane("simple-plane.obj", 1.0f);
TextureMap brickMap("texture.ppm");
vec3 lightSource = vec3(0, 0, 5);

vector<vector<float>> depthBuffer;
vector<vector<uint32_t>> frameBuffer;
int threadCount = 6;

string debugString;
std::chrono::duration<double> frameTime = std::chrono::duration<double>(1);
int renderMode = 1;

// Ran when starting program. Initializes buffers.
void initialize() {
  // Initialize a depth buffer with 0 values
  depthBuffer = vector<vector<float>>();
  for (int i = 0; i < HEIGHT; i++) {
    depthBuffer.push_back(vector<float>());
    for (int j = 0; j < WIDTH; j++) {
      depthBuffer.at(i).push_back(0.0f);
    }
  }
  // Initialize a frame buffer with black. Twice as wide as the default camera resolution
  // to accommodate a display side and a debug side.
  // This frame buffer is redundant but enables easier debugging.
  frameBuffer = vector<vector<uint32_t>>();
  for (int i = 0; i < HEIGHT; i++) {
    frameBuffer.push_back(vector<uint32_t>());
    for (int j = 0; j < WIDTH * 2; j++) {
      frameBuffer.at(i).push_back(0);
    }
  }
}

void renderDebugString(string str) {
  int yOffset = 8;
  int xOffset = 8;
  for (unsigned char character : str) {
    if (character > 127) character = '$';
    if (character == '\n') {
      yOffset += 12;
      xOffset = 8;
      continue;
    }
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        if ((font8x8_basic[character][i] >> j) & 1) {
          frameBuffer[yOffset + i][xOffset + j] = vec3ToColour(vec3(255, 255, 255), 255);
        }
      }
    }
    xOffset += 8;
  }
}

void drawFancyBackground(DrawingWindow &window) {
  // Inspired by a Sebastian Lague video, I think.
  vec3 topLeft = glm::normalize(camera.getRayDirection(0, 0));
  vec3 topRight = glm::normalize(camera.getRayDirection(WIDTH, 0));
  vec3 bottomLeft = glm::normalize(camera.getRayDirection(0, HEIGHT));
  vec3 bottomRight = glm::normalize(camera.getRayDirection(WIDTH, HEIGHT));
  vector<vec3> leftEdge = interpolate(topLeft, bottomLeft, HEIGHT);
  vector<vec3> rightEdge = interpolate(topRight, bottomRight, HEIGHT);
  for (int i = 0; i < HEIGHT; i++) {
    vector<vec3> horizontalLine = interpolate(leftEdge.at(i), rightEdge.at(i), WIDTH);
    for (int j = 0; j < WIDTH; j++) {
      frameBuffer.at(i).at(j) = vec3ToColour(horizontalLine.at(j) * 128.0f + vec3(128.0f, 128.0f, 128.0f), 255);
    }
  }
}

// Called every frame. Fills frame buffer using camera and object information, and sends to SDL wrapper.
void draw(DrawingWindow &window) {
	for (size_t y = 0; y < HEIGHT; y++) {
		for (size_t x = 0; x < WIDTH; x++) {
      depthBuffer.at(y).at(x) = 0.0f;
      frameBuffer.at(y).at(x) = 0;
		}
	}

  drawFancyBackground(window);
	vector<Object> objects = cornell.getObjects();
	switch (renderMode) {
		cout << renderMode;
		case 1: camera.rasterRender(objects, frameBuffer, depthBuffer); break;
		case 2: camera.raytraceRender(objects, frameBuffer, depthBuffer); break;
		default: camera.wireframeRender(objects, frameBuffer, depthBuffer); break;
	}

	// Apply effects
  // frameBuffer = blackAndWhite(frameBuffer);
  // frameBuffer = applyKernel(frameBuffer, boxBlurKernel);

	// Get mouse state
	int xMouse, yMouse;
  SDL_GetMouseState(&xMouse,&yMouse);

	// Print generic information
	debugString += "Mouse: " + std::to_string(xMouse) + ", " + std::to_string(yMouse) + "\n";
	uint32_t colour = frameBuffer[yMouse][xMouse];
	debugString += "RGBA: " + std::to_string((colour >> 16) & 255);
	debugString += ", " + std::to_string((colour >> 8) & 255);
	debugString += ", " + std::to_string(colour & 255);
	debugString += ", " + std::to_string((colour >> 24) & 255) + "\n";

	// Print mode-specific information
	debugString += "\n";
	switch (renderMode) {
		case 0:
			debugString += "Mode: Wireframe \n";
			debugString += "    Depth: " + std::to_string(1 / depthBuffer[yMouse][xMouse]) + "\n";
			break;
		case 1:
			debugString += "Mode: Rasterization \n";
			debugString += "    Depth: " + std::to_string(1 / depthBuffer[yMouse][xMouse]) + "\n";
			break;
		case 2:
			debugString += "Mode: Raytracing\n";
			debugString += "    Threads: " + std::to_string(threadCount) + "\n";
			break;
		default:
			debugString += "Mode: Unknown\n";
	}

	// Print orientation matrix
	debugString += '\n';
	glm::mat4 placement = camera.getPlacement();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			debugString += formatFloat(placement[i][j], 7) + " ";
		}
		debugString += "\n";
	}
	// Overlay debug string onto frame buffer
  renderDebugString(debugString);

	// Send frame buffer to SDL
  for (size_t y = 0; y < HEIGHT; y++) {
		for (size_t x = 0; x < WIDTH * 2; x++) {
      window.setPixelColour(x, y, frameBuffer.at(y).at(x));
    }
  }
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_RIGHT) camera.lookRight(2);
		if (event.key.keysym.sym == SDLK_LEFT) camera.lookLeft(2);
		if (event.key.keysym.sym == SDLK_UP) camera.lookUp(2);
		if (event.key.keysym.sym == SDLK_DOWN) camera.lookDown(2);
		if (event.key.keysym.sym == SDLK_w) camera.moveForward(0.2);
		if (event.key.keysym.sym == SDLK_s) camera.moveBack(0.2);
		if (event.key.keysym.sym == SDLK_a) camera.moveLeft(0.2);
		if (event.key.keysym.sym == SDLK_d) camera.moveRight(0.2);
    if (event.key.keysym.sym == SDLK_q) camera.moveUp(0.2);
    if (event.key.keysym.sym == SDLK_e) camera.moveDown(0.2);
    if (event.key.keysym.sym == SDLK_m) camera.toggleOrbit();
    if (event.key.keysym.sym == SDLK_n) camera.toggleLookAt();
		if (event.key.keysym.sym == SDLK_i) renderMode = 0;
		if (event.key.keysym.sym == SDLK_o) renderMode = 1;
		if (event.key.keysym.sym == SDLK_p) renderMode = 2;
		if (event.key.keysym.sym == SDLK_EQUALS) threadCount++;
		if (event.key.keysym.sym == SDLK_MINUS) if (threadCount > 1) threadCount--;
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
      if (event.button.button == SDL_BUTTON_RIGHT) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    } else if (event.button.button == SDL_BUTTON_LEFT) {
		// Deprecated debug info
    }
	}
}

// Test function for hand-checking outputs of simple functions.
void test() {
 	RayTriangleIntersection intersection = camera.getRaytracedPixelIntersection(WIDTH/2, HEIGHT/2, cornell.getObjects(), lightSource);
  cout << intersection;
	std::cout.flush();
  // std::exit(0);
}


int main(int argc, char *argv[]) {
	// srand(time(NULL));
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
  initialize();
  // Debug information
	cornell.printObjectMaterials();
  camera.lookAt(vec4(0, 0, 0, 1));
  test();
	while (true) {
		debugString = "";
		debugString += "FPS: " + std::to_string(1 / frameTime.count()) + "\n";
		auto start = std::chrono::system_clock::now();
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
    camera.update();
		draw(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
		frameTime = std::chrono::system_clock::now() - start;
	}
}
