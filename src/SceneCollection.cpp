#pragma once

#include "Scene.cpp"
#include "Camera.cpp"

class SceneCollection {
  public:
    SceneCollection() {
      sceneVect.push_back(makeCornellScene());
      sceneVect.push_back(makeTexturedCornellScene());
      sceneVect.push_back(makeSphereScene());
      sceneVect.push_back(makeLogoScene());
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
      Camera *camera2 = new Camera(vec2(320, 240), vec3(1, 1, 3));
      Scene *scene = new Scene(camera);
      Light *light = new Light("Soft", vec3(0.4, 0.4, 0.4), 0.5, Colour(255, 255, 255), true, true, 0.05);
      Light *light2 = new Light("Hard", vec3(-0.4, 0.4, 0.4), 0.5, Colour(255, 255, 255), true, false);
      scene->addAnimation(new Translation(camera, [](vec3 origin, int tick) {
        return vec3(2 * glm::cos(tick / 24.0f), 0, 2 * glm::sin(tick / 24.0f));
      }));
      camera->lookAt(new vec3(0, 0, 0));
      camera->setPosition(vec3(0, 0, 4));
      camera->changeF(-1.0f);
      vec3 *lookTarget = new vec3(0);
      camera->lookAt(lookTarget);
      camera2->lookAt(lookTarget);
      scene->addLight(light);
      scene->addLight(light2);
      scene->addObjectFile(ObjectFile("cornell-box.obj", 0.17f));
      scene->lightPositionPreview = false;
      scene->renderMode = 1;
      scene->lightingEnabled = true;
      return scene;
    }

    Scene *makeTexturedCornellScene() {
      Camera *camera = new Camera(vec2(128, 96), vec3(0, 0, 7));
      Scene *scene = new Scene(camera);
      Light *whiteLight = new Light("White", vec3(0, -0.6, 0.6), 5, Colour(255, 255, 255), true, false);
      scene->addAnimation(new Translation(whiteLight, [](vec3 origin, int tick) {
        return vec3(2 * glm::sin(tick / 24.0f), 2, 2 * glm::cos(tick / 24.0f));
      }));
      scene->getCamera()->lookAt(&(whiteLight->pos));
      scene->addLight(whiteLight);
      scene->addLight(new Light("Red", vec3(-2.2, 2.2, 3), 5, Colour(255, 0, 0), true));
      scene->addLight(new Light("Green", vec3(2.2, -2.2, 3), 5, Colour(0, 255, 0), true));
      scene->addLight(new Light("Blue", vec3(-2.2, -2.2, 3), 5, Colour(0, 0, 255), true));
      ObjectFile cornell = (ObjectFile("textured-cornell-box.obj", 1.0f));
      ObjectFile mirrorsphere = ObjectFile("mirrorsphere.obj", 1.0f);
      mirrorsphere.centerOn(vec4(1, 1, 1, 1));
      scene->addObjectFile(cornell);
      scene->addObjectFile(mirrorsphere);
      scene->lightingEnabled = true;
      scene->texturesEnabled = true;
      scene->normalMapsEnabled = true;
      scene->renderMode = 2;
      return scene;
    }

    Scene *makeSphereScene() {
      Camera *camera = new Camera(vec2(640, 480), vec3(0.05, 0.05, 3));
      Scene *scene = new Scene(camera);
      Light *whiteLight = new Light("White", vec3(0.4, 0.4, 1.5), 5, Colour(255, 255, 255), true);
      scene->addLight(whiteLight);
      ObjectFile sphere = (ObjectFile("sphere.obj", 1.0f));
      sphere.centerOn(vec4(0));
      camera->lookAt(new vec3(0, 0, 0));
      scene->addObjectFile(sphere);
      scene->lightingEnabled = true;
      scene->renderMode = 2;
      return scene;

    }

    Scene *makeLogoScene() {
      std::function<vec3(int)> cameraRotationFunction = [](int tick) {
        // For the first 5 seconds, pan camera backwards from scene as two lights race by
        if (0 <= tick && tick < 24 * 5) {
          return vec3(0, 0, 90.0 - (90.0 * ((float)tick / (24.0f * 5))));
        } else if (6 * 24 <= tick && tick < 24 * 10) {
          float positionScale = (tick - 6 * 24) / (float) (4.0 * 24);
          return vec3(-35 + 35 * positionScale, 35 - 35 * positionScale, 0);
        } else if (10 * 24 <= tick && tick < 24 * 15) {
          float positionScale = (tick - 10 * 24) / (float) (5.0 * 24);
          positionScale = std::cbrt(positionScale);
          return vec3(25 * positionScale, -25 * positionScale, 0);
        } else {
          return vec3(0, 0, 0);
        }
      };
      std::function<vec3(vec3, int)> cameraPositionFunction = [](vec3 origin, int tick) {
        // For the first 5 seconds, pan camera backwards
        if (0 <= tick && tick < 24 * 5) {
          float positionScale = (tick) / (float) (5.0 * 24);
          return vec3(0, 0, 3 + glm::pow(positionScale, 1.5f) * 2);
        } else if (24 * 5 <= tick && tick < 24 * 6){
          return vec3(0, 0, 5);
        } else if (24 * 6 <= tick && tick < 24 * 10) {
          float positionScale = (float)(tick - 24*6) / (float)(4.0f * 24);
          return vec3(-2, -2, 4 - 7 * std::pow(positionScale, 2.0f));
        } else if (10 * 24 <= tick && tick < 24 * 15) {
          float positionScale = (tick - 10 * 24) / (float) (5.0 * 24);
          positionScale = std::cbrt(positionScale);
          return vec3(2.5, 2.5, -3 + 5 * positionScale);
        }
        return vec3(2, 2, 3);
      };
      std::function<vec3(vec3, int)> whiteLightFunction = [](vec3 origin, int tick) {
        // Spiral white light inwards from second 0 onwards
        if (24 * 0 <= tick && tick < 6 * 24) {
          float positionScale = (float)(tick - 24*2) / (float)(4.0f * 24);
          float x = glm::sin(tick / 24.0f) * 2;
          float y = glm::cos(tick / 24.0f) * 2;
          return vec3(x, y, 10 - glm::pow(positionScale, 2) * 7);
        } else if (24 * 6 <= tick && tick < 24 * 10) {
          float positionScale = (float)(tick - 24*6) / (float)(4.0f * 24);
          return vec3(-2, -2, 4 - 7 * std::pow(positionScale, 2.0f));
        } else if (10 * 24 <= tick && tick < 24 * 15) {
          float positionScale = (tick - 10 * 24) / (float) (5.0 * 24);
          positionScale = std::cbrt(positionScale);
          return vec3(2.5, 2.5, -3 + 5 * positionScale);
        } else return vec3(2.5, 2.5, 3);
      };
      std::function<vec3(vec3, int)> redLightFunction = [](vec3 origin, int tick) {
        // In second 1, race light by
        if (24 * 1 <= tick && tick < 24 * 2) {
          return vec3(10 - (tick - (24 * 1)), -0.5, 2);
        }
        // And then again
        if (24 * 2 <= tick && tick < 24 * 3) {
          return vec3(-10 + (tick - (24 * 2)), -0.5, 2);
        }
        if (tick < 24 * 6) return vec3(0, 0, 500);
        float x = glm::sin(tick / 36.0f) * 3;
        float z = glm::cos(tick / 36.0f) * 3;
        return vec3(x, -0.5, z);
      };
      std::function<vec3(vec3, int)> blueLightFunction = [](vec3 origin, int tick) {
        // In second 1.5, race light by
        if (24 * 1.5f <= tick && tick < 24 * 2.5f) {
          return vec3(10 - (tick - (24 * 1.5f)), 0.5, 2);
        }
        // And then again
        if (24 * 2.5f <= tick && tick < 24 * 3.5f) {
          return vec3(-10 + (tick - (24 * 2.5f)), 0.5, 2);
        }
        if (tick < 24 * 6) return vec3(0, 0, 500);
        float x = glm::sin(tick / 24.0f) * 2.7;
        float z = glm::cos(tick / 24.0f) * 2.7;
        return vec3(x, -0.8, z);
      };
      Camera *camera = new Camera(vec2(64, 48), vec3(0.0, 0.0, 5));
      Scene *scene = new Scene(camera);
      Light *whiteLight = new Light("White", vec3(-1.5, 0, 0.9), 7, Colour(255, 255, 255), true, true, 0.1);
      Light *red = new Light("Red", vec3(1, 0, 2), 2, Colour(255, 0, 0), true, false);
      // Last minute change to green
      Light *blue = new Light("Blue", vec3(-1, 0, 2), 2, Colour(0, 255, 0), true, false);
      camera->lookAt(new vec3(0));
      scene->addLight(whiteLight);
      scene->addLight(red);
      scene->addLight(blue);
      ObjectFile logo = ObjectFile("logo.obj", 0.01f);
      logo.centerOn(vec4(0));
      ObjectFile cornellBox = ObjectFile("submission-box.obj", 1.5f);
      scene->addAnimation(new AdjustableRotation(camera, cameraRotationFunction));
      scene->addAnimation(new Translation(camera, cameraPositionFunction));
      scene->addAnimation(new Translation(whiteLight, whiteLightFunction));
      scene->addAnimation(new Translation(red, redLightFunction));
      scene->addAnimation(new Translation(blue, blueLightFunction));
      scene->addObjectFile(logo);
      scene->addObjectFile(cornellBox);
      scene->lightingEnabled = true;
      scene->texturesEnabled = true;
      scene->lightPositionPreview = false;
      scene->toggleAnimation();
      scene->normalMapsEnabled = true;
      scene->recording = false;
      scene->renderMode = 2;
      return scene;
    }

    vector<Scene*> sceneVect;
    int index = 0;
};
