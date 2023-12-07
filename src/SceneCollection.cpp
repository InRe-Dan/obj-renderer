#pragma once

#include "Scene.cpp"
#include "Camera.cpp"

class SceneCollection {
  public:
    SceneCollection() {
      sceneVect.push_back(makeLogoScene());
      sceneVect.push_back(makeCornellScene());
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
    Scene *makeCornellScene() {
      Camera *camera = new Camera(vec2(1280, 720), vec3(0, 0, 0));
      Scene *scene = new Scene(camera);
      Light *light = new Light("White", vec3(0, 0.4, 0), 0.8, Colour(255, 255, 255), true);
      scene->addAnimation(new Animation(camera, 
        [](float xStart, int tick) {return xStart + 4 * glm::sin(float(tick) / 10);},
        [](float yStart, int tick) {return yStart;},
        [](float zStart, int tick) {return zStart + 4 * glm::cos(float(tick) / 10);}));
      camera->setPosition(vec3(0, 0, 4));
      camera->changeF(-1.0f);
      vec3 *lookTarget = new vec3(0);
      camera->lookAt(lookTarget);
      scene->addLight(light);
      scene->addObjectFile(ObjectFile("cornell-box.obj", 0.17f));
      scene->lightPositionPreview = false;
      return scene;
    }

    Scene *makeTexturedCornellScene() {
      Camera *camera = new Camera();
      Scene *scene = new Scene(camera);
      Light *whiteLight = new Light("White", vec3(0, 0, 1.5), 5, Colour(255, 255, 255), true);
      scene->addAnimation(new Animation(whiteLight, 
        [](float xStart, int tick) {return xStart + 2 * glm::sin(float(tick) / 10);},
        [](float yStart, int tick) {return yStart + 2 * glm::cos(float(tick) / 10);},
        [](float zStart, int tick) {return zStart;}
      ));

      scene->getCamera()->lookAt(&(whiteLight->pos));
      scene->addLight(whiteLight);
      scene->addLight(new Light("Red", vec3(1, 1, 5), 5, Colour(255, 127, 127), true));
      scene->addLight(new Light("Green", vec3(0, 0, 5), 5, Colour(127, 255, 127), true));
      scene->addLight(new Light("Blue", vec3(-1, -1, 5), 5, Colour(127, 127, 255), true));
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

    Scene *makeLogoScene() {
      Camera *camera = new Camera(vec2(320, 180), vec3(0.0, 0.0, 5));
      Scene *scene = new Scene(camera);
      Light *whiteLight = new Light("White", vec3(0, 0, 5), 5, Colour(255, 255, 255), true);
      scene->addLight(whiteLight);
      ObjectFile logo = ObjectFile("logo.obj", 0.002f);
      logo.centerOn(vec4(0));
      scene->addObjectFile(logo);
      return scene;
    }

    vector<Scene*> sceneVect;
    int index = 0;
};
