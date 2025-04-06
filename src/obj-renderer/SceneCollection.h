#pragma once

#include "Scene.h"

class SceneCollection
{
  public:
	SceneCollection();

	Scene& getCurrent();

	void next();

	void prev();

  private:

	std::vector<Scene> sceneVect;
	size_t index = 0;

	Scene makeCornellScene();

	Scene makeTexturedCornellScene();

	Scene makeSphereScene();

	Scene makeLogoScene();

};
