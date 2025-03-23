#pragma once

#include <vector>
#include "ObjectFile.h"
#include "Light.h"

class Camera;

class Scene
{
  public:
	std::vector<ObjectFile> objectFiles;
	bool lightingEnabled = false;
	bool texturesEnabled = false;
	bool normalMapsEnabled = false;
	bool lightPositionPreview = true;
	bool smoothingEnabled = true;
	// if false, assume Phong smoothing is being used
	bool usingGouraudSmoothing = false;
	bool recording = false;
	int recordFrame = 1000;
	int renderMode = 0;

	Scene(Camera* camera);

	void addObjectFile(ObjectFile file);
	std::vector<ModelTriangle>* getModelTriangles();

	Camera* getCamera();

	Light* getControlledLight();

	void addCamera(Camera* camera);

	void addLight(Light* light);

	std::vector<Light*> getLights();
	void nextCamera();

	void prevCamera();

	void nextLight();

	void prevLight();

	int cameraCount();

	void addAnimation(Animation* a);

	void toggleAnimation();

	void update();

  private:
	std::vector<Camera*> cameras;
	std::vector<ModelTriangle> modelTriangles;
	std::vector<Light*> lights;
	std::vector<Animation*> animations;
	int cameraIndex;
	int lightIndex;
};
