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
      Camera *camera = new Camera(vec2(640, 480), vec3(0, 0, 0));
      Scene *scene = new Scene(camera);
      Light *light = new Light("White", vec3(0, 0.4, 0), 0.8, Colour(255, 255, 255), true, true, 0.03);
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
      Camera *camera = new Camera(vec2(640, 480));
      Scene *scene = new Scene(camera);
      Light *whiteLight = new Light("White", vec3(0, 0, 1.5), 5, Colour(255, 255, 255), true, true);
      scene->getCamera()->lookAt(&(whiteLight->pos));
      scene->addLight(whiteLight);
      scene->addLight(new Light("Red", vec3(1, 1, 5), 5, Colour(255, 127, 127), false));
      scene->addLight(new Light("Green", vec3(0, 0, 5), 5, Colour(127, 255, 127), false));
      scene->addLight(new Light("Blue", vec3(-1, -1, 5), 5, Colour(127, 127, 255), false));
      ObjectFile cornell = (ObjectFile("textured-cornell-box.obj", 1.0f));
      ObjectFile mirrorsphere = ObjectFile("mirrorsphere.obj", 1.0f);
      mirrorsphere.centerOn(vec4(1, 1, 1, 1));
      cornell.centerOn(vec4(0));
      scene->addObjectFile(cornell);
      scene->addObjectFile(mirrorsphere);
      return scene;
    }

    Scene *makeSphereScene() {
      Camera *camera = new Camera(vec2(640, 480), vec3(0.05, 0.05, 3));
      Scene *scene = new Scene(camera);
      Light *whiteLight = new Light("White", vec3(0.4, 0.4, 1.5), 5, Colour(255, 255, 255), true);
      scene->addLight(whiteLight);
      ObjectFile sphere = (ObjectFile("sphere.obj", 1.0f));
      sphere.centerOn(vec4(0));
      scene->addObjectFile(sphere);
      return scene;

    }

    Scene *makeLogoScene() {
      std::function<vec3(int)> cameraRotationFunction = [](int tick) {
        // For the first 5 seconds, pan camera backwards from scene as two lights race by
        if (0 <= tick && tick < 24 * 5) {
          return vec3(0, 0, 90.0 - (90.0 * ((float)tick / (24.0f * 5))));
        } else {
          return vec3(0, 0, 0);
        }
      };
      std::function<vec3(vec3, int)> cameraPositionFunction = [](vec3 origin, int tick) {
        // For the first 5 seconds, pan camera backwards
        if (0 <= tick && tick < 24 * 5) {
          return vec3(0, 0, 3 + tick * 0.04);
        }
        return vec3(0, 0, 5);
      };
      std::function<vec3(vec3, int)> whiteLightFunction = [](vec3 origin, int tick) {
        return vec3(0, 0, 4);
      };
      std::function<vec3(vec3, int)> redLightFunction = [](vec3 origin, int tick) {
        // In second 1, race light by
        if (24 * 1 <= tick && tick < 24 * 2) {
          return vec3(10 - (tick - (24 * 1)), -0.5, 2);
        }
        return vec3(0, 0, 500);
      };
      std::function<vec3(vec3, int)> blueLightFunction = [](vec3 origin, int tick) {
        // In second 1.5, race light by
        if (24 * 1.5f <= tick && tick < 24 * 2.5f) {
          return vec3(10 - (tick - (24 * 1.5f)), 0.5, 2);
        }
        return vec3(0, 0, 500);
      };
      Camera *camera = new Camera(vec2(64, 48), vec3(0.0, 0.0, 5));
      Scene *scene = new Scene(camera);
      Light *whiteLight = new Light("White", vec3(-1.5, 0, 0.9), 5, Colour(255, 255, 255), true, true);
      Light *red = new Light("Red", vec3(1, 0, 2), 3, Colour(255, 0, 0), true, false);
      Light *blue = new Light("Blue", vec3(-1, 0, 2), 3, Colour(0, 0, 255), true, false);
      scene->addLight(whiteLight);
      scene->addLight(red);
      scene->addLight(blue);
      ObjectFile logo = ObjectFile("logo.obj", 0.01f);
      logo.centerOn(vec4(0));
      ObjectFile cornellBox = ObjectFile("submission-box.obj", 1.0f);
      scene->addAnimation(new AdjustableRotation(camera, cameraRotationFunction));
      scene->addAnimation(new Translation(camera, cameraPositionFunction));
      scene->addAnimation(new Translation(whiteLight, whiteLightFunction));
      scene->addAnimation(new Translation(red, redLightFunction));
      scene->addAnimation(new Translation(blue, blueLightFunction));
      scene->addObjectFile(logo);
      scene->addObjectFile(cornellBox);
      scene->lightingEnabled = true;
      scene->texturesEnabled = true;
      scene->lightPositionPreview = true;
      scene->toggleAnimation();
      scene->normalMapsEnabled = true;
      scene->recording = false;
      scene->renderMode = 2;
      return scene;
    }

    vector<Scene*> sceneVect;
    int index = 0;
};
