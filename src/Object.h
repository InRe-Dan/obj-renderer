#pragma once

#include "CanvasTriangle.h"
#include "DrawingWindow.h"
#include "CanvasPoint.h"
#include "CanvasTriangle.h"
#include "ModelTriangle.h"

#include <fstream>
#include <vector>
#include <glm/glm.hpp>

// Class to store basic attributes about objects in ObjectFiles.
class Object
{
  public:
	Object(std::string_view id, std::shared_ptr<const Material> material);

	Object(const Object&) = delete;
	Object(Object&&) = delete;
	Object& operator=(const Object&) = delete;
	Object& operator=(Object&&) = delete;

	std::vector<ModelTriangle>& getTris()
	{
		return triangles;
	}

	const std::vector<ModelTriangle>& getTris() const
	{
		return triangles;
	}

	const Material& getMaterial() const
	{
		return *material;
	}

	void setMaterial(std::shared_ptr<const Material> material)
	{
		this->material = material;
	}

private:
	std::string name;
	std::vector<ModelTriangle> triangles;
	/// non-null
	std::shared_ptr<const Material> material;
};