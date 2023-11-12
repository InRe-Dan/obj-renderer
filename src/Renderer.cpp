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
#include "Scene.cpp"

#define WIDTH 1280
#define HEIGHT 720

using std::vector;
using glm::vec3;
using glm::vec2;

Camera primaryCamera;
Scene scene(&primaryCamera);
vector<vector<uint32_t>> upscaledFrameBuffer;

string debugString;
std::chrono::duration<double> frameTime = std::chrono::duration<double>(1);
int renderMode = 2;

// Ran when starting program. Initializes buffers.
void initialize() {
  Camera *secondaryCamera = new Camera();
  scene.addCamera(secondaryCamera);
  scene.lights.push_back(vec3(0, 0, 3));
  // scene.lights.push_back(vec3(-1, 1, 5));
  ObjectFile cornell = (ObjectFile("banana.obj", 1.0f));
  scene.addObjectFile(cornell);
  upscaledFrameBuffer = vector<vector<uint32_t>>();
  for (int i = 0; i < HEIGHT; i++) {
    upscaledFrameBuffer.push_back(vector<uint32_t>());
    for (int j = 0; j < WIDTH; j++) {
      upscaledFrameBuffer.at(i).push_back(0);
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
          upscaledFrameBuffer[yOffset + i][xOffset + j] = vec3ToColour(vec3(255, 255, 255), 255);
        }
      }
    }
    xOffset += 8;
  }
}


// Called every frame. Fills frame buffer using camera and object information, and sends to SDL wrapper.
void draw(DrawingWindow &window) {
  Camera camera = *(scene.getCamera());
	for (size_t y = 0; y < HEIGHT; y++) {
		for (size_t x = 0; x < WIDTH; x++) {
      upscaledFrameBuffer.at(y).at(x) = 0;
		}
	}

  camera.drawFancyBackground();
	switch (renderMode) {
		cout << renderMode;
		case 1: camera.rasterRender(scene); break;
		case 2: camera.raytraceRender(scene); break;
		default: camera.wireframeRender(scene); break;
	}

	// Apply effects
  arbitraryUpscale(camera.frameBuffer, upscaledFrameBuffer);
  // blackAndWhite(upscaledFrameBuffer);
  // upscaledFrameBuffer = applyKernel(upscaledFrameBuffer, sharpenKernel);
  // vector<vector<uint32_t>> hEdges = applyKernel(upscaledFrameBuffer, edgeDetectionKernelH);
  // vector<vector<uint32_t>> vEdges = applyKernel(upscaledFrameBuffer, edgeDetectionKernelV);
  // hypot(upscaledFrameBuffer, hEdges, vEdges);
  // threshold(upscaledFrameBuffer, vec3(250));
  

	// Get mouse state
	int xMouse, yMouse;
  SDL_GetMouseState(&xMouse,&yMouse);
  float upscaleFactor = WIDTH / camera.canvasWidth;
  int mouseCanvasX = glm::min(camera.canvasWidth - 1, roundI(xMouse / upscaleFactor));
  int mouseCanvasY = glm::min(camera.canvasHeight - 1, roundI(yMouse / upscaleFactor));

	// Print generic information
	debugString += "Mouse        : " + std::to_string(xMouse) + ", " + std::to_string(yMouse) + "\n";
  debugString += "Resolution   : " + std::to_string(camera.canvasWidth)+ "x" + std::to_string(camera.canvasHeight) + "\n";
	uint32_t colour = upscaledFrameBuffer.at(yMouse).at(xMouse);
	debugString += "RGBA         : " + std::to_string((colour >> 16) & 255);
	debugString += ", " + std::to_string((colour >> 8) & 255);
	debugString += ", " + std::to_string(colour & 255);
	debugString += ", " + std::to_string((colour >> 24) & 255) + "\n";
  debugString += "FOV          : " + std::to_string(roundI(glm::degrees(2 * glm::atan(camera.getImagePlaneWidth() / (2 * camera.getFocalLength()))))) + "\n";

	// Print mode-specific information
	debugString += "\n";

	switch (renderMode) {
		case 0:
			debugString += "Mode         : Wireframe \n";
			debugString += "  Depth      : " + std::to_string(1 / camera.depthBuffer[mouseCanvasY][mouseCanvasX]) + "\n";
			break;
		case 1:
			debugString += "Mode         : Rasterization \n";
			debugString += "  Depth      : " + std::to_string(1 / camera.depthBuffer[mouseCanvasY][mouseCanvasX]) + "\n";
			break;
		case 2:
			debugString += "Mode         : Raytracing\n";
    	debugString += "  Depth      : " + std::to_string(1 / camera.depthBuffer[mouseCanvasY][mouseCanvasX]) + "\n";
			debugString += "  Threads    : " + std::to_string(camera.threadCount) + "\n";
      debugString += "  Lights     : " + std::to_string(scene.lights.size()) + "\n";
      if (!(scene.lights.size() > 4)) {
        for (int i = 0; i < scene.lights.size(); i++) {

          debugString += (i == scene.lightIndex)? "   >" : "    ";
          debugString += printVec(scene.lights[i]) + "\n";
        }
      }
			break;
		default:
			debugString += "Mode: Unknown\n";
	}

	// Print orientation matrix
	debugString += '\n';
	glm::mat4 placement = camera.getPlacement();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			debugString += formatFloat(placement[i][j], 5) + " ";
		}
		debugString += "\n";
	}
	// Overlay debug string onto frame buffer
  renderDebugString(debugString);

	// Send frame buffer to SDL
  for (size_t y = 0; y < HEIGHT; y++) {
		for (size_t x = 0; x < WIDTH; x++) {
      window.setPixelColour(x, y, upscaledFrameBuffer.at(y).at(x));
    }
  }
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
    // CAMERA CONTROLS
		if (event.key.keysym.sym == SDLK_RIGHT) scene.getCamera()->lookRight(2);
		if (event.key.keysym.sym == SDLK_LEFT) scene.getCamera()->lookLeft(2);
		if (event.key.keysym.sym == SDLK_UP) scene.getCamera()->lookUp(2);
		if (event.key.keysym.sym == SDLK_DOWN) scene.getCamera()->lookDown(2);
		if (event.key.keysym.sym == SDLK_w) scene.getCamera()->moveForward(0.2);
		if (event.key.keysym.sym == SDLK_s) scene.getCamera()->moveBack(0.2);
		if (event.key.keysym.sym == SDLK_a) scene.getCamera()->moveLeft(0.2);
		if (event.key.keysym.sym == SDLK_d) scene.getCamera()->moveRight(0.2);
    if (event.key.keysym.sym == SDLK_q) scene.getCamera()->moveUp(0.2);
    if (event.key.keysym.sym == SDLK_e) scene.getCamera()->moveDown(0.2);
    if (event.key.keysym.sym == SDLK_m) scene.getCamera()->toggleOrbit();
    if (event.key.keysym.sym == SDLK_n) scene.getCamera()->toggleLookAt();
    if (event.key.keysym.sym == SDLK_z) scene.getCamera()->changeF(0.1);
    if (event.key.keysym.sym == SDLK_x) scene.getCamera()->changeF(-0.1);
    if (event.key.keysym.sym == SDLK_l) scene.nextCamera();
    if (event.key.keysym.sym == SDLK_k) scene.prevCamera();
    // MODE CONTROLS
		if (event.key.keysym.sym == SDLK_KP_1) renderMode = 0;
		if (event.key.keysym.sym == SDLK_KP_2) renderMode = 1;
		if (event.key.keysym.sym == SDLK_KP_3) renderMode = 2;
    if (event.key.keysym.sym == SDLK_KP_4) scene.lightingEnabled = !scene.lightingEnabled;
    if (event.key.keysym.sym == SDLK_KP_5) scene.texturesEnabled = !scene.texturesEnabled;
    if (event.key.keysym.sym == SDLK_KP_6) scene.normalMapsEnabled = !scene.normalMapsEnabled;
    // LIGHT CONTROLS
    if (event.key.keysym.sym == SDLK_g) scene.lights[0] += vec3(0, -0.2, 0);
    if (event.key.keysym.sym == SDLK_t) scene.lights[0] += vec3(0, 0.2, 0);
    if (event.key.keysym.sym == SDLK_f) scene.lights[0] += vec3(0.2, 0, 0);
    if (event.key.keysym.sym == SDLK_h) scene.lights[0] += vec3(-0.2, 0, 0);
    if (event.key.keysym.sym == SDLK_r) scene.lights[0] += vec3(0, 0, 0.2);
    if (event.key.keysym.sym == SDLK_y) scene.lights[0] += vec3(0, 0, -0.2);
    // GENERAL CONTROLS
    if (event.key.keysym.sym == SDLK_o) scene.getCamera()->changeResolutionBy(-32, -18);
    if (event.key.keysym.sym == SDLK_p) scene.getCamera()->changeResolutionBy(32, 18);
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
  vec3 ray = scene.getCamera()->getRayDirection(724.0f / (float(WIDTH) / float(scene.getCamera()->canvasWidth)), 250.0f / (float(HEIGHT) / float(scene.getCamera()->canvasHeight)));
 	RayTriangleIntersection intersection = scene.getCamera()->getClosestIntersection(scene.getCamera()->getPosition(), ray, scene);
  cout << intersection << "\n";
  // cout << printVec(intersection.normal) << "\n";
  RayTriangleIntersection lightIntersection = scene.getCamera()->getClosestIntersection(intersection.intersectionPoint, scene.lights[0] - intersection.intersectionPoint, scene);
  cout << lightIntersection;
  cout << lightIntersection.intersectedTriangle.colour;
	std::cout.flush();
  // std::exit(0);
}


int main(int argc, char *argv[]) {
	// srand(time(NULL));
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
  initialize();
  // Debug information
  scene.getCamera()->lookAt(vec4(0, 0, 0, 1));
  test();
	while (true) {
		debugString = "";
		debugString += "FPS: " + std::to_string(1 / frameTime.count()) + "\n";
		auto start = std::chrono::system_clock::now();
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
    scene.getCamera()->update();
		draw(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
		frameTime = std::chrono::system_clock::now() - start;
    // std::exit(0);
	}
}
