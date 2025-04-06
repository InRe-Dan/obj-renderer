#pragma once

#include "ObjectFile.h"
#include "Light.h"

#include <vector>
#include <memory>

class Camera;

class Scene
{
  public:

	explicit Scene(const Camera& camera);

	void addObjectFile(std::unique_ptr<ObjectFile>&& file);

	const std::vector<ModelTriangle>& getModelTriangles() const;

	const Camera& getCamera() const;
	Camera& getCamera();

	Light& getLight();

	void addCamera(const Camera& camera);

	void addLight(const Light& light);

	std::vector<Light>& getLights();
	const std::vector<Light>& getLights() const;

	void nextCamera();

	void prevCamera();

	void nextLight();

	void prevLight();

	size_t cameraCount() const;

  private:
	std::vector<std::unique_ptr<ObjectFile>> objectFiles;
	std::vector<Camera> cameras;
	std::vector<ModelTriangle> modelTriangles;
	std::vector<Light> lights;
	size_t cameraIndex = 0;
	size_t lightIndex = 0;
};
