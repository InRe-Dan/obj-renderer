#pragma once

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
#include "drawing.cpp"
#include <thread>


using std::vector;
using glm::vec3;
using glm::vec2;
using glm::round;

// Object to represent a camera in a scene.
class Camera {
  public:
    int canvasWidth;
    int canvasHeight;
    // Takes parameters for camera resolution.
    Camera(int w, int h) {
      threadCount = 6;
      canvasHeight = h;
      canvasWidth = w;
      raytracingImagePlaneWidth = 5.0f;
      focalLength = 2;
      placement = glm::mat4(1, 0, 0, -0.8, 0, 1, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0);
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
      vec3 imagePlaneTopLeft = forward * focalLength + (up * (canvasHeight * 0.5f * pixelLength)) + (-right * (pixelLength * canvasWidth * 0.5f));
      return imagePlaneTopLeft + float(x) * pixelLength * right + float(y) * -up * pixelLength;
    }

    RayTriangleIntersection getClosestIntersection(vec3 ray, vector<Object> objects, vec3 rayOrigin) {
      vec3 closestSolution = vec3(-HUGE_VAL, 0, 0);
      vec3 closestPoint;
      vec3 point;
      ModelTriangle solutionT;
      solutionT.colour = Colour(0, 0, 0);
      int solutionIndex = -1; // Represents No Solution
      int i = 0;
      for (Object object : objects) {
        for (ModelTriangle triangle : object.triangles) {
          vec3 e0 = vec3(triangle.vertices[1] - triangle.vertices[0]);
          vec3 e1 = vec3(triangle.vertices[2] - triangle.vertices[0]);
          vec3 SPVector = rayOrigin - vec3(triangle.vertices[0]);
          glm::mat3 DEMatrix(-ray, e0, e1);
          vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
          if (glm::abs(possibleSolution.x) < glm::abs(closestSolution.x)) {
            point = vec3(solutionT.vertices[0]) + vec3(solutionT.vertices[1] - solutionT.vertices[0]) * closestSolution.y + vec3(solutionT.vertices[2] - solutionT.vertices[0]) * closestSolution[1];
            if (0.0f <= possibleSolution.y && possibleSolution.y <= 1.0f && 0.0f <= possibleSolution.z && possibleSolution.z <= 1.0f && possibleSolution.y + possibleSolution.z <= 1.0f && possibleSolution.x < 0.0f) {
              closestPoint = point;
              closestSolution = possibleSolution;
              solutionT = triangle;
              solutionIndex = i;
            }
          }
          i++;
        }
      }
      float distance = closestSolution.x;
      size_t index = solutionIndex;
      RayTriangleIntersection intersection(closestPoint, distance, solutionT, index);
      return intersection;
    }

    RayTriangleIntersection getRaytracedPixelIntersection(int xPos, int yPos, vector<Object> objects, vec3 lightSource) {
      vec3 rayDirection = getRayDirection(xPos, yPos);
      RayTriangleIntersection intersection = getClosestIntersection(rayDirection, objects, getPosition());
      /* if (intersection.triangleIndex == -1) return intersection;
      vec3 pointToLight = -(intersection.intersectionPoint - lightSource);
      RayTriangleIntersection lightIntersection = getClosestIntersection(pointToLight, objects, lightSource);
      if (lightIntersection.triangleIndex != -1 && glm::abs(lightIntersection.distanceFromCamera) < glm::length(pointToLight)) {
        Colour c = intersection.intersectedTriangle.colour;
        intersection.intersectedTriangle.colour = Colour(c.red/2, c.green/2, c.blue/2);
      } */
      return intersection;
    }

    void raytraceSection(int x1, int x2, int y1, int y2, vector<vector<uint32_t>> *frameBuffer, vector<Object> *objects, vec3 *lightSource) {
      for (int i = y1; i < y2; i++) {
        for (int j = x1; j < x2; j++) {
          RayTriangleIntersection intersection = getRaytracedPixelIntersection(j, i, *objects, *lightSource);
			    ModelTriangle t = intersection.intersectedTriangle;
          if (intersection.triangleIndex == -1) continue;
			    (*frameBuffer).at(i).at(j) = vec3ToColour(vec3(t.colour.red, t.colour.green, t.colour.blue), 255);
        }
      }
    }

    void rasterRender(vector<Object> objects, ObjectFile cornell, vector<vector<uint32_t>> &frameBuffer, vector<vector<float>> &depthBuffer) {
      for (Object object : objects) {
        for (ModelTriangle triangle : object.triangles) {
          CanvasPoint a = getCanvasIntersectionPoint(glm::vec3(triangle.vertices.at(0)));
          CanvasPoint b = getCanvasIntersectionPoint(glm::vec3(triangle.vertices.at(1)));
          CanvasPoint c = getCanvasIntersectionPoint(glm::vec3(triangle.vertices.at(2)));
          if (isInBounds(a, vec4(0, 0, canvasWidth, canvasHeight)) && isInBounds(b, vec4(0, 0, canvasWidth, canvasHeight)) && isInBounds(c, vec4(0, 0, canvasWidth, canvasHeight))) {
            CanvasTriangle canvasTriangle(a, b, c);
            filledTriangle(canvasTriangle, cornell.getKdOf(object), frameBuffer, depthBuffer);
          }
        }
      }
    }

    void wireframeRender(vector<Object> objects, ObjectFile cornell, vector<vector<uint32_t>> &frameBuffer, vector<vector<float>> &depthBuffer) {
      for (Object object : objects) {
        for (ModelTriangle triangle : object.triangles) {
          CanvasPoint a = getCanvasIntersectionPoint(glm::vec3(triangle.vertices.at(0)));
          CanvasPoint b = getCanvasIntersectionPoint(glm::vec3(triangle.vertices.at(1)));
          CanvasPoint c = getCanvasIntersectionPoint(glm::vec3(triangle.vertices.at(2)));
          if (isInBounds(a, vec4(0, 0, canvasHeight, canvasWidth)) && isInBounds(b, vec4(0, 0, canvasWidth, canvasHeight)) && isInBounds(c, vec4(0, 0, canvasWidth, canvasHeight))) {
            CanvasTriangle canvasTriangle(a, b, c);
            strokedTriangle(canvasTriangle, cornell.getKdOf(object), frameBuffer, depthBuffer);
          }
        }
      }
    }

    void raytraceRender(vector<Object> objects, ObjectFile cornell, vector<vector<uint32_t>> &frameBuffer, vector<vector<float>> &depthBuffer, vec3 lightSource) {
      vector<std::thread> threadVect;
      int slice_height = canvasHeight / threadCount;
      for (int i = 0; i < threadCount - 1; i++) {
        threadVect.push_back(std::thread(&Camera::raytraceSection, this, 0, canvasWidth, slice_height * i, slice_height * (i + 1), &frameBuffer, &objects, &lightSource));
      }
      threadVect.push_back(std::thread(&Camera::raytraceSection, this, 0, canvasWidth, slice_height * (threadCount - 1), canvasHeight, &frameBuffer, &objects, &lightSource));
      for (int i = 0; i < threadVect.size(); i++) {
        threadVect.at(i).join();
      }
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
      placement = getYRotationMatrix(-degrees) * placement;
    }

    void lookLeft(float degrees) {
      placement = getYRotationMatrix(degrees) * placement;
    }

    void changeF(float diff) {
      focalLength += diff;
    }

    private:
      bool isOrbiting;
      bool isLooking;
      vec4 lookTarget;
      float focalLength;
      float raytracingImagePlaneWidth;
      glm::mat4 placement;
      int threadCount;
};