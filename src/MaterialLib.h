#pragma once

#include "Material.h"

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
#include <iostream>
#include <unordered_map>

// Class used to load and encapsulate a .mtl file.
class MaterialLib
{
  public:
	std::unordered_map<std::string, Material> materials;
	MaterialLib();

	MaterialLib(std::string filename);

  private:
	std::string file;
};