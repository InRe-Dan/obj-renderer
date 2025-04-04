#pragma once

#include "Material.h"

#include "CanvasTriangle.h"
#include "DrawingWindow.h"
#include "CanvasPoint.h"
#include "CanvasTriangle.h"
#include "Colour.h"
#include "Utils.h"
#include "TextureMap.h"
#include "ModelTriangle.h"


#include <fstream>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <GLM/glm.hpp>

// Class used to load and encapsulate a .mtl file.
class MaterialLib
{
  public:
	MaterialLib();

	MaterialLib(const std::filesystem::path& file);

	std::shared_ptr<const Material> get(std::string_view name) const
	{
		return materials.at(std::string(name));
	}

	std::shared_ptr<const Material> getDefault() const
	{
		return get("default");
	}

  private:
	std::unordered_map<std::string, std::shared_ptr<Material>> materials;
};