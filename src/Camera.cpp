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
#include "RayTriangleIntersection.h"

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
        vec3 newPos = vec3(getYRotationMatrix(3) * vec4(pos, 1));
        placement = glm::mat4(vec4(0, 0, 0, newPos.x), vec4(0, 0, 0, newPos.y), vec4(0, 0, 0, newPos.z), vec4(0, 0, 0 , 0)) + (placement + getTranslationMatrix(-pos));
      }
      if (isLooking) {
        vec3 forward = glm::normalize(vec3(lookTarget) - getPosition());
        vec3 right = glm::normalize(glm::cross(vec3(forward), vec3(0, 1, 0)));
        vec3 up = glm::normalize(glm::cross(vec3(forward), vec3(right)));
        // Not flipping the rightways vector seems to flip the scene... ¯\_(ツ)_/¯
        vec3 pos = getPosition();
        placement = glm::mat4(vec4(right, pos.x), vec4(-up, pos.y), vec4(-forward, pos.z), vec4(0, 0, 0, 1));
      }
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

    RayTriangleIntersection getClosestIntersection(int xPos, int yPos, vector<Object> objects) {
      RayTriangleIntersection intersection;
      vec3 rayDirection;
      vec3 closestSolution = vec3(INFINITY, 0, 0);
      ModelTriangle solutionTriangle;

      for (Object object : objects) {
        for (ModelTriangle triangle : object.triangles) {
          glm::vec3 e0 = vec3(triangle.vertices[1]) - vec3(triangle.vertices[0]);
          glm::vec3 e1 = vec3(triangle.vertices[2]) - vec3(triangle.vertices[0]);
          glm::vec3 SPVector = getPosition() - vec3(triangle.vertices[0]);
          glm::mat3 DEMatrix(-rayDirection, e0, e1);
          glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
          if (possibleSolution.x < closestSolution.x) {
            closestSolution = possibleSolution;
            solutionTriangle = triangle;
          }
        }
      }
      return intersection;
    }

    void moveBy(vec3 vect) {
      // placement[3] += vect;
      placement[0][3] += vect.x;
      placement[1][3] += vect.y;
      placement[2][3] += vect.z;
    }

    void moveRight(float a) {
      moveBy(-a * glm::normalize(vec3(placement[0])));
    }

    void moveLeft(float a) {
      moveRight(-a);
    }

    void moveForward(float a) {
      moveBy(-a * glm::normalize(vec3(placement[2])));
    }

    void moveBack(float a) {
      moveForward(-a);
    }

    void lookUp(float degrees) {
      placement = getXRotationMatrix(-degrees) * placement;
    }

    void lookDown(float degrees) {
      placement = getXRotationMatrix(degrees) * placement;
    }

    void lookRight(float degrees) {
      placement = getYRotationMatrix(-2) * placement;
    }

    void lookLeft(float degrees) {
      placement = getYRotationMatrix(2) * placement;
    }

    void changeF(float diff) {
      focalLength += diff;
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