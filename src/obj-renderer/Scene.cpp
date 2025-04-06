#include "Scene.h"

#include "ObjectFile.h"
#include "Light.h"
#include "Camera.h"

#include <vector>

Scene::Scene(const Camera& camera)
{
	cameras.push_back(camera);
}

void Scene::addObjectFile(std::unique_ptr<ObjectFile>&& file)
{
	objectFiles.push_back(std::move(file));
	for (const std::unique_ptr<Object>& object : objectFiles.back()->getObjects())
	{
		const std::vector<ModelTriangle>& tris = object->getTris();
		modelTriangles.insert(modelTriangles.end(), tris.begin(), tris.end());
	}
}

const std::vector<ModelTriangle>& Scene::getModelTriangles() const
{
	return modelTriangles;
}

const Camera& Scene::getCamera() const
{
	return cameras.at(cameraIndex);
}

Camera& Scene::getCamera()
{
	return cameras.at(cameraIndex);
}


Light& Scene::getLight()
{
	return lights.at(lightIndex);
}

void Scene::addCamera(const Camera& camera)
{
	cameras.push_back(camera);
}

void Scene::addLight(const Light& light)
{
	lights.push_back(light);
}

const std::vector<Light>& Scene::getLights() const
{
	return lights;
}

std::vector<Light>& Scene::getLights()
{
	return lights;
}

void Scene::nextCamera()
{
	if (cameraIndex == cameras.size() - 1)
		cameraIndex = 0;
	else
		cameraIndex++;
}

void Scene::prevCamera()
{
	if (cameraIndex == 0)
		cameraIndex = cameras.size() - 1;
	else
		cameraIndex--;
}

void Scene::nextLight()
{
	if (lightIndex == lights.size() - 1)
		lightIndex = 0;
	else
		lightIndex++;
}

void Scene::prevLight()
{
	if (lightIndex == 0)
		lightIndex = lights.size() - 1;
	else
		lightIndex--;
}

size_t Scene::cameraCount() const
{
	return cameras.size();
}
