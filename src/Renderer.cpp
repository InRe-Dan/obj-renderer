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

#define WIDTH 640
#define HEIGHT 480
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

vector<vector<float>> depthBuffer;
vector<vector<uint32_t>> frameBuffer;

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

void renderDebugString(string str, DrawingWindow window) {
  int yOffset = 20;
  int xOffset = 20;
  for (unsigned char character : str) {
    if (character > 127) character = '$';
    if (character == '\n') {
      yOffset += 10;
      xOffset = 20;
      continue;
    }
    cout << "drawing " << character << "\n";
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        if ((font8x8_basic[character][i] >> j) & 1) {
          cout << "(" << yOffset + i << ", " << xOffset + j << ")\n";
          frameBuffer[yOffset + i][xOffset + j] = vec3ToColour(vec3(255, 255, 255), 255);
        }
      }
    }
    xOffset += 8;
  }
}

// Print debug information about the depth and colour of a clicked pixel
void leftClickedOn(int x, int y) {
  cout << "====================" << '\n';
  if (x > WIDTH) (x -= WIDTH);
  cout << "Pixel Clicked: " << x << ", " << y << '\n';
  cout << "R: " <<  ((frameBuffer.at(y).at(x) & 0x00FF0000) >> 16) << '\n';
  cout << "G: " <<  ((frameBuffer.at(y).at(x) & 0x0000FF00) >> 8) << '\n';
  cout << "B: " <<  (frameBuffer.at(y).at(x) & 0x000000FF) << '\n';
  cout << "Depth: " << depthBuffer.at(y).at(x) << "\n";
  cout << "====================" << '\n';
}

// Draw a line from a point to another on a window.
void line(CanvasPoint to, CanvasPoint from, Colour colour, DrawingWindow &window) {
	float xDiff = to.x - from.x;
	float yDiff = to.y - from.y;
	float steps = round(glm::max(glm::abs(xDiff), glm::abs(yDiff)));
	vector<CanvasPoint> interpolation = bindToRectangle(interpolate(to, from, steps), vec2(1, 1), vec2(WIDTH-2, HEIGHT-2));
	vec3 colVect(colour.red, colour.green, colour.blue);
  for (CanvasPoint point : interpolation) {
    if (depthBuffer.at(round(point.y)).at(round(point.x)) < 1 / point.depth) {
      depthBuffer.at(round(point.y)).at(round(point.x)) = 1 / point.depth;
			frameBuffer.at(round(point.y)).at(round(point.x)) = vec3ToColour(colVect, 255);
    }
  }
}

// Draw a wireframe triangle
void strokedTriangle(CanvasTriangle &triangle, Colour colour, DrawingWindow &window) {
	line(triangle.v0(), triangle.v1(), colour, window);
	line(triangle.v1(), triangle.v2(), colour, window);
	line(triangle.v2(), triangle.v0(), colour, window);
}

// Draw a triangle which has a flat top or flat bottom. This is intended as a helper function.
void rasterizeTriangle(CanvasPoint point, CanvasPoint base1, CanvasPoint base2, Colour colour, DrawingWindow &window) {
	assert(round(base1.y) == round(base2.y));
	vector<CanvasPoint> pointToOne = interpolate(point, base1, ceil(abs(point.y - base1.y)));
	vector<CanvasPoint> pointToTwo = interpolate(point, base2, ceil(abs(point.y - base2.y)));
	for (int i = 0; i < ceil(pointToOne.size()); i++) {
		CanvasPoint to = pointToOne.at(i);
		CanvasPoint from = pointToTwo.at(i);
		line(to, from, colour, window);
	}
}

// Draw a full triangle
void filledTriangle(CanvasTriangle &triangle, Colour colour, DrawingWindow &window) {
	// Sort vertices by height
	CanvasPoint top = triangle.v0();
	CanvasPoint mid = triangle.v1();
	CanvasPoint bot = triangle.v2();
	if (top.y > mid.y) std::swap(top, mid);
	if (mid.y > bot.y) std::swap(mid, bot);
	if (top.y > mid.y) std::swap(top, mid);
	top = CanvasPoint(round(top.x), round(top.y), top.depth);
	mid = CanvasPoint(round(mid.x), round(mid.y), mid.depth);
	bot = CanvasPoint(round(bot.x), round(bot.y), bot.depth);


	// Locate point at the same y level from middle vertex
	int height = round(glm::abs(top.y - bot.y));
	vector<CanvasPoint> topToBot = interpolate(top, bot, height);

  // Create an imaginary point at the same height as the middle point, along the midpoint of top and bottom
	float imaginaryY = mid.y;
	float imaginaryX = mid.x;
  float imaginaryDepth = mid.depth;
	for (int i = 0; i < height; i++) {
		if (topToBot.at(i).y == mid.y) {
			imaginaryY = round(topToBot.at(i).y);
			imaginaryX = round(topToBot.at(i).x);
      imaginaryDepth = topToBot.at(i).depth;
			break;
		}
	}
	CanvasPoint imaginary(imaginaryX, imaginaryY, imaginaryDepth);

  // Call helper function to rasterize a triangle in parts from left to right, top to bottom.
	if (mid.y == top.y) {
		rasterizeTriangle(bot, top, mid, colour, window);
	} 
	else if (mid.y == bot.y) {
		rasterizeTriangle(top, mid, bot, colour, window);
	} else {
		rasterizeTriangle(top, mid, imaginary, colour, window);
		rasterizeTriangle(bot, mid, imaginary, colour, window);
	}
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
	vector<CanvasPoint> topToBot = interpolate(top, bot, height);
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
	vector<CanvasPoint> topToImaginary = interpolate(top, imaginary, topHeight);
	vector<CanvasPoint> topToMid = interpolate(top, mid, topHeight);
	vector<CanvasPoint> imaginaryToBot = interpolate(imaginary, bot, botHeight);
	vector<CanvasPoint> midToBot = interpolate(mid, bot, botHeight);

	vector<vec2> topToImaginaryT = interpolate(top.texturePoint, imaginary.texturePoint, topHeight);
	vector<vec2> topToMidT = interpolate(top.texturePoint, mid.texturePoint, topHeight);
	vector<vec2> imaginaryToBotT = interpolate(imaginary.texturePoint, bot.texturePoint, botHeight);
	vector<vec2> midToBotT = interpolate(mid.texturePoint, bot.texturePoint, botHeight);
	// Colour between top two lines
	for (int i = 0; i < topHeight; i++) {
		CanvasPoint to = topToImaginary.at(i);
		CanvasPoint from = topToMid.at(i);
		vec2 toT = topToImaginaryT.at(i);
		vec2 fromT = topToMidT.at(i);
		int width = abs(to.x - from.x);
		vector<CanvasPoint> interpolated = interpolate(to, from, width);
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
		CanvasPoint to = imaginaryToBot.at(i);
		CanvasPoint from = midToBot.at(i);
		vec2 toT = imaginaryToBotT.at(i);
		vec2 fromT = midToBotT.at(i);
		int width = abs(to.x - from.x);
		vector<CanvasPoint> interpolated = interpolate(to, from, width);
		vector<vec2> interpolatedT = interpolate(toT, fromT, width);
		for (int j = 0; j < width; j++) {
			int x = round(interpolated.at(j).x);
			int y = round(interpolated.at(j).y);
			int xT = round(interpolatedT.at(j).x);
			int yT = round(interpolatedT.at(j).y);
      frameBuffer.at(y).at(x) = map.pixels.at(yT * map.width + xT);
		}
	}
	strokedTriangle(triangle, WHITE, window);
}

// Called every frame. Fills frame buffer using camera and object information, and sends to SDL wrapper.
void draw(DrawingWindow &window) {
	for (size_t y = 0; y < HEIGHT; y++) {
		for (size_t x = 0; x < WIDTH; x++) {
      depthBuffer.at(y).at(x) = 0.0f;
      frameBuffer.at(y).at(x) = 0;
		}
	}
	vector<Object> objects = cornell.getObjects();
	for (Object object : objects) {
		for (ModelTriangle triangle : object.triangles) {
			CanvasPoint a = camera.getCanvasIntersectionPoint(glm::vec3(triangle.vertices.at(0)));
			CanvasPoint b = camera.getCanvasIntersectionPoint(glm::vec3(triangle.vertices.at(1)));
			CanvasPoint c = camera.getCanvasIntersectionPoint(glm::vec3(triangle.vertices.at(2)));
      if (isInBounds(a, vec4(0, 0, WIDTH, HEIGHT)) && isInBounds(b, vec4(0, 0, WIDTH, HEIGHT)) && isInBounds(c, vec4(0, 0, WIDTH, HEIGHT))) {
		  	CanvasTriangle canvasTriangle(a, b, c);
			  filledTriangle(canvasTriangle, cornell.getKdOf(object), window);
      }
      // strokedTriangle(canvasTriangle, cornell.getKdOf(object), window);
		}
	}
  renderDebugString("Hello World!", window);
  float min = 999999;
  float max = 0;
  for (size_t y = 0; y < HEIGHT; y++) {
		for (size_t x = 0; x < WIDTH; x++) {
      if (depthBuffer.at(y).at(x) < 0.0005 && depthBuffer.at(y).at(x) > -0.0005);
      else if (1 / depthBuffer.at(y).at(x) < min) min = 1 / depthBuffer.at(y).at(x);
      else if (1 / depthBuffer.at(y).at(x) > max) max = 1 / depthBuffer.at(y).at(x);
    }
  }
  float range = max - min;
  for (size_t y = 0; y < HEIGHT; y++) {
		for (size_t x = 0; x < WIDTH; x++) {
      float position = (((1 / depthBuffer.at(y).at(x)) - min)/range);
      uint8_t luminance = round(position * 255);
      uint32_t col = 255;
      col <<= 8;
      col += luminance;
      col <<= 8;
      col += luminance;
      col <<= 8;
      col += luminance;
      // if (luminance) cout << depthBuffer.at(y).at(x)<< '\n';
			frameBuffer.at(y).at(x + WIDTH) = col;
		}
	}
  
  for (size_t y = 0; y < HEIGHT; y++) {
		for (size_t x = 0; x < WIDTH * 2; x++) {
      window.setPixelColour(x, y, frameBuffer.at(y).at(x));
    }
  }
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_RIGHT) camera.moveBy(vec3(-0.2, 0, 0));
		else if (event.key.keysym.sym == SDLK_LEFT) camera.moveBy(vec3(+0.2, 0, 0));
		else if (event.key.keysym.sym == SDLK_UP) camera.moveBy(vec3(0, 0.2, 0));
		else if (event.key.keysym.sym == SDLK_DOWN) camera.moveBy(vec3(0, -0.2, 0));
		else if (event.key.keysym.sym == SDLK_w) camera.moveBy(vec3(0, 0, -0.2));
		else if (event.key.keysym.sym == SDLK_s) camera.moveBy(vec3(0, 0, +0.2));
		else if (event.key.keysym.sym == SDLK_a) camera.changeF(-0.2);
		else if (event.key.keysym.sym == SDLK_d) camera.changeF(0.2);
    else if (event.key.keysym.sym == SDLK_m) camera.toggleOrbit();
    else if (event.key.keysym.sym == SDLK_n) camera.toggleLookAt();
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
      if (event.button.button == SDL_BUTTON_RIGHT) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    } else if (event.button.button == SDL_BUTTON_LEFT) {
      leftClickedOn(event.button.x, event.button.y);
    }
	}
}

// Test function for hand-checking outputs of simple functions.
void test() {
	vec3 one(255, 0, 0);
	vec3 two(0, 255, 0);
	vector<vec3> thing = interpolate(one, two, 3);
	std::cout.flush();
	quick_exit(0);
}


int main(int argc, char *argv[]) {
	// srand(time(NULL));
	DrawingWindow window = DrawingWindow(WIDTH * 2, HEIGHT, false);
	SDL_Event event;
	// test();
  initialize();
  // Debug information
	cornell.printObjectMaterials();
  camera.lookAt(vec4(0, 0, 0, 1));
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
    camera.update();
		draw(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
