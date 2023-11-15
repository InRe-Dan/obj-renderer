#pragma once

#include <vector>
#include "ObjectFile.cpp"
#include "Light.cpp"

class Camera;

class Scene {
  public:
    vector<ObjectFile> objectFiles;
    bool lightingEnabled = false;
    bool texturesEnabled = false;
    bool normalMapsEnabled = false;

    Scene(Camera *camera) {
      objectFiles = vector<ObjectFile>();
      lights = vector<Light*>();
      cameras = vector<Camera*>();
      cameras.push_back(camera);
      cameraIndex = 0;
      lightIndex = 0;
      modelTriangles = vector<ModelTriangle>();
    }

    void addObjectFile(ObjectFile file) {
      objectFiles.push_back(file);
      for (ObjectFile objectFile : objectFiles) {
        for (Object object : objectFile.getObjects()) {
          for (ModelTriangle t : object.triangles) {
            modelTriangles.push_back(t);
          }
        }
      }

    }
    vector<ModelTriangle> *getModelTriangles() {
      return &modelTriangles;
    }

    Camera *getCamera() {return cameras.at(cameraIndex);}

    Light *getControlledLight() {return lights.at(lightIndex);}

    void addCamera(Camera *camera) {
      cameras.push_back(camera);
    }

    void addLight(Light *light) {
      lights.push_back(light);
    }

    vector<Light*> getLights() {
      return lights;
    }

    void nextCamera() {
      if (cameraIndex == cameras.size() - 1) cameraIndex = 0;
      else cameraIndex++;
    }

    void prevCamera() {
      if (cameraIndex == 0) cameraIndex = cameras.size() - 1;
      else cameraIndex--;
    }

    void nextLight() {
      if (lightIndex == lights.size() - 1) lightIndex = 0;
      else lightIndex++;
    }

    void prevLight() {
      if (lightIndex == 0) lightIndex = lights.size() - 1;
      else lightIndex--;
    }

    int cameraCount() {
      return cameras.size();
    }

  private:
    vector<Camera*> cameras;
    vector<ModelTriangle> modelTriangles;
    int cameraIndex;
    int lightIndex;
    vector<Light*> lights;
};