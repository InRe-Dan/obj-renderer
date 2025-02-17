#include "drawing.h"

#include "vectors.h"

#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <Utils.h>
#include <TextureMap.h>
#include <ModelTriangle.h>
#include <CanvasTriangle.h>

#include "Colour.h"

#include <fstream>
#include <vector>

#include <glm/glm.hpp>

using std::vector;
using glm::vec3;
using glm::vec2;

void line(
	CanvasPoint to,
	CanvasPoint from,
	Colour colour,
	vector<vector<uint32_t>>& frameBuffer,
	vector<vector<float>>& depthBuffer)
{
	float xDiff = to.x - from.x;
	float yDiff = to.y - from.y;
	float steps = round(glm::max(glm::abs(xDiff), glm::abs(yDiff)));
	vector<CanvasPoint> interpolation = bindToRectangle(
		interpolate(to, from, steps),
		vec2(1, 1),
		vec2(frameBuffer.at(0).size() - 1, frameBuffer.size() - 1));
	vec3 colVect(colour.red, colour.green, colour.blue);
	for (CanvasPoint point : interpolation)
	{
		if (depthBuffer.at(roundI(point.y)).at(roundI(point.x)) < point.depth)
		{
			depthBuffer.at(roundI(point.y)).at(roundI(point.x)) = point.depth;
			frameBuffer.at(roundI(point.y)).at(roundI(point.x)) =
				vec3ToColour(colVect, 255);
		}
	}
}

void strokedTriangle(
	CanvasTriangle& triangle,
	Colour colour,
	vector<vector<uint32_t>>& frameBuffer,
	vector<vector<float>>& depthBuffer)
{
	line(triangle.v0(), triangle.v1(), colour, frameBuffer, depthBuffer);
	line(triangle.v1(), triangle.v2(), colour, frameBuffer, depthBuffer);
	line(triangle.v2(), triangle.v0(), colour, frameBuffer, depthBuffer);
}

void rasterizeTriangle(
	CanvasPoint point,
	CanvasPoint base1,
	CanvasPoint base2,
	Colour colour,
	vector<vector<uint32_t>>& frameBuffer,
	vector<vector<float>>& depthBuffer)
{
	assert(roundI(base1.y) == roundI(base2.y));
	vector<CanvasPoint> pointToOne =
		interpolate(point, base1, ceil(abs(point.y - base1.y)));
	vector<CanvasPoint> pointToTwo =
		interpolate(point, base2, ceil(abs(point.y - base2.y)));
	for (int i = 0; i < ceil(pointToOne.size()); i++)
	{
		CanvasPoint to = pointToOne.at(i);
		CanvasPoint from = pointToTwo.at(i);
		line(to, from, colour, frameBuffer, depthBuffer);
	}
}

// Draw a full triangle
void filledTriangle(
	CanvasTriangle& triangle,
	Colour colour,
	vector<vector<uint32_t>>& frameBuffer,
	vector<vector<float>>& depthBuffer)
{
	// Sort vertices by height
	CanvasPoint top = triangle.v0();
	CanvasPoint mid = triangle.v1();
	CanvasPoint bot = triangle.v2();
	if (top.y > mid.y)
		std::swap(top, mid);
	if (mid.y > bot.y)
		std::swap(mid, bot);
	if (top.y > mid.y)
		std::swap(top, mid);
	top = CanvasPoint(roundI(top.x), roundI(top.y), top.depth);
	mid = CanvasPoint(roundI(mid.x), roundI(mid.y), mid.depth);
	bot = CanvasPoint(roundI(bot.x), roundI(bot.y), bot.depth);

	// Locate point at the same y level from middle vertex
	int height = roundI(glm::abs(top.y - bot.y));
	vector<CanvasPoint> topToBot = interpolate(top, bot, height);

	// Create an imaginary point at the same height as the middle point, along
	// the midpoint of top and bottom
	float imaginaryY = mid.y;
	float imaginaryX = mid.x;
	float imaginaryDepth = mid.depth;
	for (int i = 0; i < height; i++)
	{
		if (topToBot.at(i).y == mid.y)
		{
			imaginaryY = roundI(topToBot.at(i).y);
			imaginaryX = roundI(topToBot.at(i).x);
			imaginaryDepth = topToBot.at(i).depth;
			break;
		}
	}
	CanvasPoint imaginary(imaginaryX, imaginaryY, imaginaryDepth);

	// Call helper function to rasterize a triangle in parts from left to right,
	// top to bottom.
	if (mid.y == top.y)
	{
		rasterizeTriangle(bot, top, mid, colour, frameBuffer, depthBuffer);
	}
	else if (mid.y == bot.y)
	{
		rasterizeTriangle(top, mid, bot, colour, frameBuffer, depthBuffer);
	}
	else
	{
		rasterizeTriangle(
			top,
			mid,
			imaginary,
			colour,
			frameBuffer,
			depthBuffer);
		rasterizeTriangle(
			bot,
			mid,
			imaginary,
			colour,
			frameBuffer,
			depthBuffer);
	}
}

void circle(
	int x,
	int y,
	float proportionalRad,
	float depth,
	Colour colour,
	std::vector<std::vector<uint32_t>>& frameBuffer,
	std::vector<std::vector<float>>& depthBuffer)
{
	int rad = roundI(frameBuffer.at(0).size() * proportionalRad);
	int lowerx = glm::max(0, x - rad);
	int higherx = glm::min(int(frameBuffer.at(0).size()), x + rad);
	int lowery = glm::max(0, y - rad);
	int highery = glm::min(int(frameBuffer.size()), y + rad);
	float distanceFromRadius = 0.0;
	float alpha;
	vec3 resultColour;
	for (int i = lowery; i < highery; i++)
	{
		for (int j = lowerx; j < higherx; j++)
		{
			distanceFromRadius = glm::length(vec2(x, y) - vec2(j, i));
			if (distanceFromRadius <= rad)
			{
				if (depthBuffer[i][j] < depth)
				{
					// https://stackoverflow.com/questions/17283485/apply-an-alpha-to-a-color-mathematically
					// Blend with background quadratically at the edges of the
					// circle
					alpha = 1.0f - glm::pow(distanceFromRadius / rad, 4);
					resultColour =
						(intColToVec3(frameBuffer[i][j]) * (1 - alpha)) +
						(vec3(colour.red, colour.green, colour.blue) * alpha);
					frameBuffer[i][j] = vec3ToColour(resultColour, 255);
				}
			}
		}
	}
}
