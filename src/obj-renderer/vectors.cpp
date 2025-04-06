#include "DrawingWindow.h"
#include "Utils.h"
#include "TextureMap.h"
#include "vectors.h"
#include "ModelTriangle.h"

#include <functional>
#include <iomanip>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

glm::mat4 getXRotationMatrix(float degrees)
{
	return glm::mat4(
		1,
		0,
		0,
		0,
		0,
		glm::cos(glm::radians(degrees)),
		-glm::sin(glm::radians(degrees)),
		0,
		0,
		glm::sin(glm::radians(degrees)),
		glm::cos(glm::radians(degrees)),
		0,
		0,
		0,
		0,
		1);
}

glm::mat4 getYRotationMatrix(float degrees)
{
	return glm::mat4(
		glm::cos(glm::radians(degrees)),
		0,
		glm::sin(glm::radians(degrees)),
		0,
		0,
		1,
		0,
		0,
		-glm::sin(glm::radians(degrees)),
		0,
		glm::cos(glm::radians(degrees)),
		0,
		0,
		0,
		0,
		1);
}

glm::mat4 getZRotationMatrix(float degrees)
{
	return glm::mat4(
		glm::cos(glm::radians(degrees)),
		-glm::sin(glm::radians(degrees)),
		0,
		0,
		glm::sin(glm::radians(degrees)),
		glm::cos(glm::radians(degrees)),
		0,
		0,
		0,
		0,
		1,
		0,
		0,
		0,
		0,
		1);
}

glm::mat4 getTranslationMatrix(glm::vec3 vect)
{
	glm::mat4 matrix(0);
	matrix[0][3] = vect.x;
	matrix[1][3] = vect.y;
	matrix[2][3] = vect.z;
	return matrix;
}

std::vector<glm::vec3> generateLightOffsets()
{
	static std::vector<glm::vec3> lightOffsets;
	if (lightOffsets.size() != 0)
	{
		return lightOffsets;
	}
	for (int i = 0; i < 20; i++)
	{
		float randx = (float) (rand()) / (float) (RAND_MAX);
		float randy = (float) (rand()) / (float) (RAND_MAX);
		float randz = (float) (rand()) / (float) (RAND_MAX);
		lightOffsets.push_back(glm::vec3(randx, randy, randz));
	}
	return lightOffsets;
}

// Clip a std::vector of canvas points such that all points are within some bounds
void bindToRectangle(std::vector<glm::vec2>& input, glm::vec2 topLeft, glm::vec2 bottomRight)
{
	std::erase_if(input, [&topLeft, &bottomRight](const glm::vec2 point)
		{
			if (topLeft.x > point.x)
				return false;
			if (bottomRight.x < point.x)
				return false;
			if (topLeft.y > point.y)
				return false;
			if (bottomRight.y < point.y)
				return false;
			return true;
		});
}