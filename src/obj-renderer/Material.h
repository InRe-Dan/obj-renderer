#pragma once

#include "vectors.h"
#include "TextureMap.h"

#include <vector>
#include <map>
#include <string_view>
#include <optional>
#include <glm/glm.hpp>


class Material
{
  public:

	Material(std::string_view name = "default");

	void setDiffuseColour(glm::vec4 colour);

	void setAmbientColour(glm::vec4 colour);

	void setSpecularColour(glm::vec4 colour);

	void setSpecularExponent(float value);

	void setDiffuseMap(const Surface& map);

	void setBumpMap(const Surface& map);

	void setReflectivity(float refl)
	{
		reflectivity = refl;
	}

	std::optional<glm::vec4> getDiffuseColour() const;

	std::optional<glm::vec4> getAmbientColour() const;

	std::optional<glm::vec4> getSpecularColour() const;

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
	std::optional<glm::vec4> diffuse;
	bool ambientDefined = false;
	std::optional<glm::vec4> ambient;
	std::optional<glm::vec4> specular;
	float specularExponent = 0;
	std::optional<Surface> diffuseTexture;
	std::optional<Surface> normalTexture;
	/// Postprocessed bumpmap
	std::vector<std::vector<glm::vec3>> bump_vectors;
};