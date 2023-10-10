#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <TextureMap.h>
#include <ModelTriangle.h>
#include "vecutil.cpp"

using std::vector;
using glm::vec3;
using glm::vec2;
using glm::round;

// Object to represent a camera in a scene.
class Camera {
  public:
    // Takes parameters for camera resolution.
    Camera(int w, int h) {
      canvasHeight = h;
      canvasWidth = w;
      focalLength = 2;
      placement = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 10, 0, 0, 0, 0);
    }
    void setOrbit(bool set) {
      isOrbiting = set;
    }
    void toggleOrbit() {
      isOrbiting = !isOrbiting;
    }
    void lookAt(vec4 target) {
      lookTarget = target;
    }
    void toggleLookAt() {
      isLooking = !isLooking;
    }
    void update() {
      if (isOrbiting) {
        vec3 pos = getPosition();
        placement = getTranslationMatrix(pos) * getYRotationMatrix(2) * getTranslationMatrix(-pos) * placement;
      }
      if (isLooking) {
        vec3 forward = glm::normalize(vec3(lookTarget) - getPosition());
        vec3 right = glm::normalize(glm::cross(vec3(forward), vec3(0, 1, 0)));
        vec3 up = glm::normalize(glm::cross(vec3(forward), vec3(right)));
        // Not flipping the rightways vector seems to flip the scene... ¯\_(ツ)_/¯
        vec3 pos = getPosition();
        placement = glm::mat4(vec4(-right, pos.x), vec4(up, pos.y), vec4(forward, pos.z), vec4(0, 0, 0, 1));
      }
      printPlacement();
    }
    glm::mat4 getPlacement() {
      return placement;
    }
    vec3 getPosition() {
      return vec3(placement[0][3], placement[1][3], placement[2][3]);
    }
    glm::mat3 getOrientation() {
      return glm::mat3(placement);
    }
    CanvasPoint getCanvasIntersectionPoint(vec3 vertexLocation) {
      // All coordinates are relative to the camera!
      vec3 vertexToCamera = (vertexLocation - getPosition()) * getOrientation();
      float u = focalLength * (vertexToCamera.x/vertexToCamera.z) * 50 + canvasWidth/2;
      float v = focalLength * (vertexToCamera.y/vertexToCamera.z) * 50 + canvasHeight/2;
      return CanvasPoint(u, v, glm::length(vertexToCamera));
    }

    void moveBy(vec3 vect) {
      // placement[3] += vect;
      placement[0][3] += vect.x;
      placement[1][3] += vect.y;
      placement[2][3] += vect.z;
    }

    void changeF(float diff) {
      focalLength += diff;
    }

    void printPlacement() {
      cout << "======== CAMERA PLACEMENT ======" << "\n";
      for (int i = 0; i < 4; i++ ) {
        for (int j = 0; j < 4; j++) {
          cout << placement[i][j] << " ";
        }
        cout << "\n";
      }
      cout << "================================" << "\n";
    }

    private:
      int canvasWidth;
      int canvasHeight;
      bool isOrbiting;
      bool isLooking;
      vec4 lookTarget;
      float focalLength;
      glm::mat4 placement;
};