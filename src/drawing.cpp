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
#include "vecutil.cpp"

// Draw a line from a point to another on a window.
void line(CanvasPoint to, CanvasPoint from, Colour colour, vector<vector<uint32_t>> &frameBuffer, vector<vector<float>> &depthBuffer) {
	float xDiff = to.x - from.x;
	float yDiff = to.y - from.y;
	float steps = round(glm::max(glm::abs(xDiff), glm::abs(yDiff)));
	vector<CanvasPoint> interpolation = bindToRectangle(interpolate(to, from, steps), vec2(1, 1), vec2(frameBuffer.at(0).size() - 1, frameBuffer.size() - 1));
	vec3 colVect(colour.red, colour.green, colour.blue);
  for (CanvasPoint point : interpolation) {
    if (depthBuffer.at(roundI(point.y)).at(roundI(point.x)) < point.depth) {
      depthBuffer.at(roundI(point.y)).at(roundI(point.x)) = point.depth;
			frameBuffer.at(roundI(point.y)).at(roundI(point.x)) = vec3ToColour(colVect, 255);
    }
  }
}

// Draw a wireframe triangle
void strokedTriangle(CanvasTriangle &triangle, Colour colour, vector<vector<uint32_t>> &frameBuffer, vector<vector<float>> &depthBuffer) {
	line(triangle.v0(), triangle.v1(), colour, frameBuffer, depthBuffer);
	line(triangle.v1(), triangle.v2(), colour, frameBuffer, depthBuffer);
	line(triangle.v2(), triangle.v0(), colour, frameBuffer, depthBuffer);
}

// Draw a triangle which has a flat top or flat bottom. This is intended as a helper function.
void rasterizeTriangle(CanvasPoint point, CanvasPoint base1, CanvasPoint base2, Colour colour, vector<vector<uint32_t>> &frameBuffer, vector<vector<float>> &depthBuffer) {
	assert(roundI(base1.y) == roundI(base2.y));
	vector<CanvasPoint> pointToOne = interpolate(point, base1, ceil(abs(point.y - base1.y)));
	vector<CanvasPoint> pointToTwo = interpolate(point, base2, ceil(abs(point.y - base2.y)));
	for (int i = 0; i < ceil(pointToOne.size()); i++) {
		CanvasPoint to = pointToOne.at(i);
		CanvasPoint from = pointToTwo.at(i);
		line(to, from, colour, frameBuffer, depthBuffer);
	}
}

// Draw a full triangle
void filledTriangle(CanvasTriangle &triangle, Colour colour, vector<vector<uint32_t>> &frameBuffer, vector<vector<float>> &depthBuffer) {
	// Sort vertices by height
	CanvasPoint top = triangle.v0();
	CanvasPoint mid = triangle.v1();
	CanvasPoint bot = triangle.v2();
	if (top.y > mid.y) std::swap(top, mid);
	if (mid.y > bot.y) std::swap(mid, bot);
	if (top.y > mid.y) std::swap(top, mid);
	top = CanvasPoint(roundI(top.x), roundI(top.y), top.depth);
	mid = CanvasPoint(roundI(mid.x), roundI(mid.y), mid.depth);
	bot = CanvasPoint(roundI(bot.x), roundI(bot.y), bot.depth);


	// Locate point at the same y level from middle vertex
	int height = roundI(glm::abs(top.y - bot.y));
	vector<CanvasPoint> topToBot = interpolate(top, bot, height);

  // Create an imaginary point at the same height as the middle point, along the midpoint of top and bottom
	float imaginaryY = mid.y;
	float imaginaryX = mid.x;
  float imaginaryDepth = mid.depth;
	for (int i = 0; i < height; i++) {
		if (topToBot.at(i).y == mid.y) {
			imaginaryY = roundI(topToBot.at(i).y);
			imaginaryX = roundI(topToBot.at(i).x);
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

// Draw a circle to the frame buffer. Takes a "proportional" radius, which is a fraction of the screen size. 
// Will not draw pixels if the circle depth is higher than what is already in the buffer.
// Intended to be used for visualizing positions of lights and cameras in post-processing.
void circle(int x, int y, float proportionalRad, float depth, Colour colour, vector<vector<uint32_t>> &frameBuffer, vector<vector<float>> depthBuffer ) {
	int rad = roundI(frameBuffer.at(0).size() * proportionalRad);
	int lowerx = glm::max(0, x - rad);
	int higherx = glm::min(int(frameBuffer.at(0).size()), x + rad);
	int lowery = glm::max(0, y - rad);
	int highery = glm::min(int(frameBuffer.size()), y + rad);
	float distanceFromRadius;
	float alpha;
	vec3 resultColour;
	for (int i = lowery; i < highery; i++) {
		for (int j = lowerx; j < higherx; j++) {
			distanceFromRadius = glm::length(vec2(x, y) - vec2(j, i));
			if (distanceFromRadius <= rad) {
				if (depthBuffer[i][j] < depth) {
					// https://stackoverflow.com/questions/17283485/apply-an-alpha-to-a-color-mathematically
          // Blend with background quadratically at the edges of the circle
					alpha = 1.0f - glm::pow(distanceFromRadius/rad, 4);
					resultColour = (intColToVec3(frameBuffer[i][j]) * (1 - alpha)) + (vec3(colour.red, colour.green, colour.blue) * alpha);
					frameBuffer[i][j] = vec3ToColour(resultColour, 255);
				}
			}
		}
	}
}

/*
This is old code to create a textured triangle in the raster render. Certainly won't work anymore, but will be useful for debugging a new method.

void texturedTriangle(CanvasTriangle &triangle, TextureMap &map, vector<vector<uint32_t>> &frameBuffer, vector<vector<float>> &depthBuffer) {
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
*/