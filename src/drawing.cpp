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
#include <vecutil.cpp>

// Draw a line from a point to another on a window.
void line(CanvasPoint to, CanvasPoint from, Colour colour, vector<vector<uint32_t>> &frameBuffer, vector<vector<uint32_t>> &depthBuffer) {
	float xDiff = to.x - from.x;
	float yDiff = to.y - from.y;
	float steps = round(glm::max(glm::abs(xDiff), glm::abs(yDiff)));
	vector<CanvasPoint> interpolation = interpolate(to, from, steps);
	vec3 colVect(colour.red, colour.green, colour.blue);
  for (CanvasPoint point : interpolation) {
    if (depthBuffer.at(round(point.y)).at(round(point.x)) < 1 / point.depth) {
      depthBuffer.at(round(point.y)).at(round(point.x)) = 1 / point.depth;
			frameBuffer.at(round(point.y)).at(round(point.x)) = vec3ToColour(colVect, 255);
    }
  }
}

// Draw a wireframe triangle
void strokedTriangle(CanvasTriangle &triangle, Colour colour, vector<vector<uint32_t>> &frameBuffer, vector<vector<uint32_t>> &depthBuffer) {
	line(triangle.v0(), triangle.v1(), colour, frameBuffer, depthBuffer);
	line(triangle.v1(), triangle.v2(), colour, frameBuffer, depthBuffer);
	line(triangle.v2(), triangle.v0(), colour, frameBuffer, depthBuffer);
}

// Draw a triangle which has a flat top or flat bottom. This is intended as a helper function.
void rasterizeTriangle(CanvasPoint point, CanvasPoint base1, CanvasPoint base2, Colour colour, vector<vector<uint32_t>> &frameBuffer, vector<vector<uint32_t>> &depthBuffer) {
	assert(round(base1.y) == round(base2.y));
	vector<CanvasPoint> pointToOne = interpolate(point, base1, ceil(abs(point.y - base1.y)));
	vector<CanvasPoint> pointToTwo = interpolate(point, base2, ceil(abs(point.y - base2.y)));
	for (int i = 0; i < ceil(pointToOne.size()); i++) {
		CanvasPoint to = pointToOne.at(i);
		CanvasPoint from = pointToTwo.at(i);
		line(to, from, colour, frameBuffer, depthBuffer);
	}
}

// Draw a full triangle
void filledTriangle(CanvasTriangle &triangle, Colour colour, vector<vector<uint32_t>> &frameBuffer, vector<vector<uint32_t>> &depthBuffer) {
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
		rasterizeTriangle(bot, top, mid, colour, frameBuffer, depthBuffer);
	} 
	else if (mid.y == bot.y) {
		rasterizeTriangle(top, mid, bot, colour, frameBuffer, depthBuffer);
	} else {
		rasterizeTriangle(top, mid, imaginary, colour, frameBuffer, depthBuffer);
		rasterizeTriangle(bot, mid, imaginary, colour, frameBuffer, depthBuffer);
	}
}

void texturedTriangle(CanvasTriangle &triangle, TextureMap &map, vector<vector<uint32_t>> &frameBuffer, vector<vector<uint32_t>> &depthBuffer) {
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
			// window.setPixelColour(x, y, map.pixels.at(yT * map.width + xT));
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
	strokedTriangle(triangle, Colour(255, 255, 255), frameBuffer, depthBuffer);
}