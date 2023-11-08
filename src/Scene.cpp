#pragma once

#include <vector>
#include "ObjectFile.cpp"

class Camera;

class Scene {
  public:
      int lightIndex;
      vector<ObjectFile> objectFiles;

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
      int objectFileIndex = 0;
      for (ObjectFile objectFile : objectFiles) {
        int objectIndex = 0;
        for (Object object : objectFile.getObjects()) {
          for (ModelTriangle t : object.triangles) {
            modelTriangles.push_back(t);
            modelTriangleRootObjectFile.push_back(objectFileIndex);
            modelTriangleRootObject.push_back(objectIndex);
          }
          objectIndex++;
        }
        objectFileIndex++;
      }

    }
    vector<ModelTriangle> *getModelTriangles() {
      return &modelTriangles;
    }
    Material *getMaterial(int index) {
      string matName = (objectFiles[modelTriangleRootObjectFile[index]]).getObjects()[modelTriangleRootObject[index]].material;
      Material *mat = &objectFiles[modelTriangleRootObjectFile[index]].matLib->getMaterials()[matName];
      return mat;
    }
    vector<vec3> lights;
    Camera *getCamera() {return cameras.at(cameraIndex);}

  private:
    vector<Camera*> cameras;
    vector<ModelTriangle> modelTriangles;
    vector<int> modelTriangleRootObjectFile;
    vector<int> modelTriangleRootObject;
    int cameraIndex;
};