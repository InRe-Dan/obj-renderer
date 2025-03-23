#include "Scene.h"

#include "ObjectFile.h"
#include "Light.h"
#include "Camera.h"

#include <vector>

using std::vector;

Scene::Scene(Camera* camera)
{
	objectFiles = vector<ObjectFile>();
	lights = vector<Light*>();
	cameras = vector<Camera*>();
	cameras.push_back(camera);
	cameraIndex = 0;
	lightIndex = 0;
	modelTriangles = vector<ModelTriangle>();
}

void Scene::addObjectFile(ObjectFile file)
{
	objectFiles.push_back(file);
	for (ObjectFile objectFile : objectFiles)
	{
		for (Object object : objectFile.getObjects())
		{
			for (ModelTriangle t : object.triangles)
			{
				modelTriangles.push_back(t);
			}
		}
	}
}
vector<ModelTriangle>* Scene::getModelTriangles()
{
	return &modelTriangles;
}

Camera* Scene::getCamera()
{
	return cameras.at(cameraIndex);
}

Light* Scene::getControlledLight()
{
	return lights.at(lightIndex);
}

void Scene::addCamera(Camera* camera)
{
	cameras.push_back(camera);
}

void Scene::addLight(Light* light)
{
	lights.push_back(light);
}

vector<Light*> Scene::getLights()
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

int Scene::cameraCount()
{
	return cameras.size();
}

void Scene::addAnimation(Animation* a)
{
	animations.push_back(a);
}

void Scene::toggleAnimation()
{
	for (Animation* a : animations)
	{
		a->toggle();
	}
}

void Scene::update()
{
	for (Animation* a : animations)
	{
		a->animate();
	}
}
