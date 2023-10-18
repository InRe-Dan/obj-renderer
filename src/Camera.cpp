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
      raytracingImagePlaneWidth = 5.0f;
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
      return CanvasPoint(u, v, -vertexToCamera.z);
    }

    vec3 getRayDirection(int x, int y) {
      glm::mat3 o = getOrientation();
      vec3 right = glm::normalize(o[0]);
      vec3 up = - glm::normalize(o[1]);
      vec3 forward = glm::normalize(o[2]);
      float pixelLength = raytracingImagePlaneWidth / canvasWidth;
      vec3 imagePlaneTopLeft = forward * focalLength + (up * raytracingImagePlaneWidth * 0.5f * float(canvasHeight / canvasWidth)) + (-right  * 0.5f * raytracingImagePlaneWidth);
      return imagePlaneTopLeft + float(x) * pixelLength * right + float(y) * -up * pixelLength;
    }

    bool validatePointInTriangle(vec3 s, ModelTriangle t) {
      // https://math.stackexchange.com/a/28552
      // https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates.html
      // https://gamedev.stackexchange.com/a/23745
      vec3 v0 = vec3(t.vertices[1] - t.vertices[0]);
      vec3 v1 = vec3(t.vertices[2] - t.vertices[1]);
      vec3 v2 = s - vec3(t.vertices[0]);
      float d00 = glm::dot(v0, v0);
      float d01 = glm::dot(v0, v1);
      float d11 = glm::dot(v1, v1);
      float d20 = glm::dot(v2, v0);
      float d21 = glm::dot(v2, v1);
      float denom = d00 * d11 - d01 * d01;
      float v = (d11 * d20 - d01 * d21) / denom;
      float w = (d00 * d21 - d01 * d20) / denom;
      float u = 1.0f - v - w;
      cout << "Point " << printVec(vec3(u, w, v)) << " is in " << printVec(vec3(t.vertices[0])) << "? ";
      if (v <= 0.0f) return false;
      if (v >= 1.0f) return false;
      if (w <= 0.0f) return false;
      if (w >= 1.0f) return false;
      if (u <= 0.0f) return false;
      if (u >= 1.0f) return false;
      cout << "Yes! ";
      return true;

      
      // vec3 A = vec3(t.vertices[0]);
      // vec3 B = vec3(t.vertices[1]);
      // vec3 C = vec3(t.vertices[2]);
      // vec3 point = vec3(t.vertices[0]) + vec3(t.vertices[1] - t.vertices[0]) * s.y + vec3(t.vertices[2] - t.vertices[0]) * s.z;
      // cout << "Is point " << printVec(point) << "? ";
      // float area = glm::length(glm::cross(B - A, C - A)) / 2;
      // float alpha = glm::length(glm::cross(point - B, point - C)) / 2 * area;
      // float beta = glm::length(glm::cross(point - C, point - A)) / 2 * area;
      // float gamma = 1 - alpha - beta;
      // cout << printVec(vec3(alpha, beta, gamma)) << ". ";
      // if (0 <= alpha && alpha <= 1 && 0 <= beta && beta <= 1 && 0 <= gamma && gamma <= 1) {cout << "Yes. "; 
      // return true;};
      // return false;
    }

    RayTriangleIntersection getClosestIntersection(int xPos, int yPos, vector<Object> objects) {
      vec3 rayDirection = getRayDirection(xPos, yPos);
      // cout << "Ray shot at " + printVec(rayDirection) << "\n";
      vec3 closestSolution = vec3(HUGE_VAL, 0, 0);
      vec3 closestPoint;
      vec3 point;
      ModelTriangle solutionT;
      solutionT.colour = Colour(0, 0, 0);

      for (Object object : objects) {
        for (ModelTriangle triangle : object.triangles) {
          // cout << "Testing " << triangle.colour << " triangle. ";
          vec3 e0 = vec3(triangle.vertices[1] - triangle.vertices[0]);
          vec3 e1 = vec3(triangle.vertices[2] - triangle.vertices[0]);
          vec3 SPVector = getPosition() - vec3(triangle.vertices[0]);
          glm::mat3 DEMatrix(-rayDirection, e0, e1);
          vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
          // cout << "Possible solution: " << printVec(possibleSolution) << ". ";
          if (glm::abs(possibleSolution.x) < glm::abs(closestSolution.x)) {
            point = vec3(solutionT.vertices[0]) + vec3(solutionT.vertices[1] - solutionT.vertices[0]) * closestSolution.y + vec3(solutionT.vertices[2] - solutionT.vertices[0]) * closestSolution[1];
            if (0.0f <= possibleSolution.y && possibleSolution.y <= 1.0f && 0.0f <= possibleSolution.z && possibleSolution.z <= 1.0f) {
              closestPoint = point;
              closestSolution = possibleSolution;
              solutionT = triangle;
            }
          }
          // cout << "\n";
        }
      }

      float distance = closestSolution.x;
      size_t index = 3; // random number cause the program crashes with 0
      RayTriangleIntersection intersection(closestPoint, distance, solutionT, index);

      return intersection;
    }

    void moveBy(vec3 vect) {
      // placement[3] += vect;
      placement[0][3] += vect.x;
      placement[1][3] += vect.y;
      placement[2][3] += vect.z;
    }

    void moveUp(float a) {
      moveBy(a * glm::normalize(vec3(placement[1])));
    }

    void moveDown(float a) {
      moveUp(-a);
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
      float raytracingImagePlaneWidth;
      glm::mat4 placement;
};