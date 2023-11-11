#pragma once

#include <vector>
#include "ObjectFile.cpp"

class Camera;

class Scene {
  public:
    int lightIndex;
    vector<ObjectFile> objectFiles;
    bool lightingEnabled = false;
    bool texturesEnabled = false;
    bool normalMapsEnabled = false;
    vector<vec3> lights;

    Scene(Camera *camera) {
      objectFiles = vector<ObjectFile>();
      lights = vector<vec3>();
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

    void nextCamera() {
      if (cameraIndex == cameras.size() - 1) cameraIndex = 0;
      else cameraIndex++;
    }

    void prevCamera() {
      if (cameraIndex == 0) cameraIndex = cameras.size() - 1;
      else cameraIndex--;
    }

  private:
    vector<Camera*> cameras;
    vector<ModelTriangle> modelTriangles;
    int cameraIndex;
};