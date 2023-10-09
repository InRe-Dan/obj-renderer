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

class Camera {
  public:
    Camera(int w, int h) {
      canvasHeight = h;
      canvasWidth = w;
      position = vec3(0.0, 0.0, 8.0);
      focalLength = 2;
      orientation = glm::mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    }
    void setOrbit(bool set) {
      isOrbiting = set;
    }
    void toggleOrbit() {
      isOrbiting = !isOrbiting;
    }
    void lookAt(vec3 target) {
      lookTarget = target;
    }
    void toggleLookAt() {
      isLooking = !isLooking;
    }
    void update() {
      if (isOrbiting) {
        position = getYRotationMatrix(2) * position;
      }
      if (isLooking) {
        vec3 forward = glm::normalize(lookTarget - position);
        vec3 right = glm::normalize(glm::cross(forward, vec3(0, 1, 0)));
        vec3 up = glm::normalize(glm::cross(forward, right));
        // Not flipping the rightways vector seems to flip the scene... ¯\_(ツ)_/¯
        orientation = glm::mat3(-right, up, forward);
      }
    }
    glm::mat3 getOrientation() {
      return orientation;
    }
    vec3 getPosition() {
      return position;
    }
    CanvasPoint getCanvasIntersectionPoint(vec3 vertexLocation) {
      // All coordinates are relative to the camera!
      vec3 vertexToCamera = (vertexLocation - position) * orientation;
      float u = focalLength * (vertexToCamera.x/vertexToCamera.z) * 50 + canvasWidth/2;
      float v = focalLength * (vertexToCamera.y/vertexToCamera.z) * 50 + canvasHeight/2;
      return CanvasPoint(u, v, glm::length(vertexToCamera));
    }

    void moveBy(vec3 vect) {
      position += vect;
    }

    void changeF(float diff) {
      focalLength += diff;
    }

    private:
      int canvasWidth;
      int canvasHeight;
      bool isOrbiting;
      bool isLooking;
      vec3 lookTarget;
      vec3 position;
      float focalLength;
      glm::mat3 orientation;
};