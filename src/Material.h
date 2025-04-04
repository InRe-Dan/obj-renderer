#pragma once

#include "utility/vectors.h"


#include "CanvasTriangle.h"
#include "TextureMap.h"

#include <vector>
#include <map>
#include <string_view>
#include <optional>
#include <GLM/glm.hpp>


// Class to store information about a material in a MaterialFile. Shouldn't
// really be used alone.
class Material
{
  public:

	Material(std::string_view name = "default");

	void setDiffuseColour(Colour colour);

	void setAmbientColour(Colour colour);

	void setSpecularColour(Colour colour);

	void setSpecularExponent(float value);

	void setDiffuseMap(const Surface& map);

	void setBumpMap(const Surface& map);

	void setReflectivity(float refl)
	{
		reflectivity = refl;
	}

	std::optional<Colour> getDiffuseColour() const;

	std::optional<Colour> getAmbientColour() const;

	std::optional<Colour> getSpecularColour() const;

	float getSpecularExponent() const;

	const std::optional<Surface>& getDiffuseTexture() const;

	const std::optional<Surface>& getNormalTexture() const;

	bool getReflectivity () const
	{
		return reflectivity;
	}

  private:
	std::string name = "default";
	/// Normalized value - 1.0 means light is reflected and this material is a mirror.
	float reflectivity = 0.0;
	std::optional<Colour> diffuse;
	bool ambientDefined = false;
	std::optional<Colour> ambient;
	std::optional<Colour> specular;
	float specularExponent = 0;
	std::optional<Surface> diffuseTexture;
	std::optional<Surface> normalTexture;
	/// Postprocessed bumpmap
	std::vector<std::vector<glm::vec3>> bump_vectors;
};