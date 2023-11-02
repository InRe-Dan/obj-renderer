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
    vector<ModelTriangle> getModelTriangles() {
      return modelTriangles;
    }
    vector<vec3> lights;
    Camera *getCamera() {return cameras.at(cameraIndex);}

  private:
    vector<ObjectFile> objectFiles;
    vector<Camera*> cameras;
    vector<ModelTriangle> modelTriangles;
    int cameraIndex;
};