#pragma once

#include "vectors.h"

#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <Utils.h>
#include <TextureMap.h>
#include <ModelTriangle.h>
#include <CanvasTriangle.h>

#include <fstream>
#include <vector>

#include <glm/glm.hpp>


// Draw a line from a point to another on a window.
void line(
	CanvasPoint to,
	CanvasPoint from,
	Colour colour,
	std::vector<std::vector<uint32_t>>& frameBuffer,
	std::vector<std::vector<float>>& depthBuffer);

// Draw a wireframe triangle
void strokedTriangle(
	CanvasTriangle& triangle,
	Colour colour,
	std::vector<std::vector<uint32_t>>& frameBuffer,
	std::vector<std::vector<float>>& depthBuffer);

// Draw a triangle which has a flat top or flat bottom. This is intended as a
// helper function.
void rasterizeTriangle(
	CanvasPoint point,
	CanvasPoint base1,
	CanvasPoint base2,
	Colour colour,
	std::vector<std::vector<uint32_t>>& frameBuffer,
	std::vector<std::vector<float>>& depthBuffer);

// Draw a full triangle
void filledTriangle(
	CanvasTriangle& triangle,
	Colour colour,
	std::vector<std::vector<uint32_t>>& frameBuffer,
	std::vector<std::vector<float>>& depthBuffer);

// Draw a circle to the frame buffer. Takes a "proportional" radius, which is a
// fraction of the screen size. Will not draw pixels if the circle depth is
// higher than what is already in the buffer. Intended to be used for
// visualizing positions of lights and cameras in post-processing.
void circle(
	int x,
	int y,
	float proportionalRad,
	float depth,
	Colour colour,
	std::vector<std::vector<uint32_t>>& frameBuffer,
	std::vector<std::vector<float>>& depthBuffer);