#pragma once

#include "Scene.cpp"
#include "Camera.cpp"

class SceneCollection {
  public:
    SceneCollection() {
      sceneVect.push_back(makeTexturedCornellScene());
      sceneVect.push_back(makeSphereScene());
    }

    Scene *getCurrent() {
      return sceneVect.at(index);
    }

    void next() {
      if (index == sceneVect.size() - 1) index = 0;
      else index++;
    }

    void prev() {
      if (index == 0) index = sceneVect.size() - 1;
      else index--;
    }
  private:
    Scene *makeTexturedCornellScene() {
      Camera *camera = new Camera();  Camera *secondaryCamera = new Camera();
      Scene *scene = new Scene(camera);
      Light *whiteLight = new Light("White", vec3(0, 0, 1.5), 5, Colour(255, 255, 255), true);
      scene->addAnimation(new Animation(whiteLight, 
      [](float xStart, int tick) {return xStart + 1 * glm::sin(float(tick) / 10);},
      [](float yStart, int tick) {return yStart + 1 * glm::cos(float(tick) / 10);},
      [](float zStart, int tick) {return zStart;}
      ));
      /* 
      scene.addAnimation(new Animation(scene.getCamera(), 
      [](float xStart, int tick) {return xStart + 0.5 * glm::sin(float(tick) / 10);},
      [](float yStart, int tick) {return yStart;},
      [](float zStart, int tick) {return zStart;}
      ));
      */
      scene->getCamera()->lookAt(&(whiteLight->pos));
      scene->addLight(whiteLight);
      scene->addLight(new Light("Red", vec3(1, 1, 5), 5, Colour(255, 127, 127), true));
      scene->addLight(new Light("Green", vec3(0, 0, 5), 5, Colour(127, 255, 127), true));
      scene->addLight(new Light("Blue", vec3(-1, -1, 5), 5, Colour(127, 127, 255), true));
      // scene.lights.push_back(vec3(-1, 1, 5));
      ObjectFile cornell = (ObjectFile("textured-cornell-box.obj", 1.0f));
      cornell.centerOn(vec4(0));
      scene->addObjectFile(cornell);
      return scene;
    }

    Scene *makeSphereScene() {
      Camera *camera = new Camera(vec2(320, 180), vec3(0.05, 0.05, 3));
      Scene *scene = new Scene(camera);
      Light *whiteLight = new Light("White", vec3(0.4, 0.4, 1.5), 5, Colour(255, 255, 255), true);
      scene->addAnimation(new Animation(whiteLight, 
      [](float xStart, int tick) {return xStart + 1 * glm::sin(float(tick) / 10);},
      [](float yStart, int tick) {return yStart + 1 * glm::cos(float(tick) / 10);},
      [](float zStart, int tick) {return zStart;}
      ));
      scene->addLight(whiteLight);
      ObjectFile sphere = (ObjectFile("sphere.obj", 1.0f));
      sphere.centerOn(vec4(0));
      scene->addObjectFile(sphere);
      return scene;

    }

    vector<Scene*> sceneVect;
    int index = 0;
};