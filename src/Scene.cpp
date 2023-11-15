#pragma once

#include <vector>
#include "ObjectFile.cpp"
#include "Light.cpp"

class Camera;

class Scene {
  public:
    int lightIndex;
    vector<ObjectFile> objectFiles;
    bool lightingEnabled = false;
    bool texturesEnabled = false;
    bool normalMapsEnabled = false;
    vector<Light> lights;

    Scene(Camera *camera) {
      objectFiles = vector<ObjectFile>();
      lights = vector<Light>();
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

    void addCamera(Camera *camera) {
      cameras.push_back(camera);
    }

    void nextCamera() {
      if (cameraIndex == cameras.size() - 1) cameraIndex = 0;
      else cameraIndex++;
    }

    void prevCamera() {
      if (cameraIndex == 0) cameraIndex = cameras.size() - 1;
      else cameraIndex--;
    }

    int cameraCount() {
      return cameras.size();
    }

  private:
    vector<Camera*> cameras;
    vector<ModelTriangle> modelTriangles;
    int cameraIndex;
};