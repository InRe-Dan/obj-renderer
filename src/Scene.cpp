#pragma once

#include <vector>
#include "ObjectFile.cpp"

class Camera;

class Scene {
  public:
    Scene(Camera *camera) {
      objectFiles = vector<ObjectFile>();
      lights = vector<vec3>();
      cameras = vector<Camera*>();
      cameras.push_back(camera);
      cameraIndex = 0;
    }

    vector<ObjectFile> objectFiles;
    vector<vec3> lights;
    Camera *getCamera() {return cameras.at(cameraIndex);}

  private:
    vector<Camera*> cameras;
    int cameraIndex;
};