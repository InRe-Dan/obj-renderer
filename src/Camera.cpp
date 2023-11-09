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
#include "Scene.cpp"

using std::vector;
using glm::vec3;
using glm::vec2;

// Object to represent a camera in a scene.
class Camera {
  public:
    int threadCount;
    int canvasWidth;
    int canvasHeight;
    vector<vector<uint32_t>> frameBuffer;
    vector<vector<float>> depthBuffer;
    // Takes parameters for camera resolution.
    Camera(int w, int h) {
      threadCount = 6;
      canvasHeight = h;
      canvasWidth = w;
      raytracingImagePlaneWidth = 5.0f;
      focalLength = 2;
      placement = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 5, 0, 0, 0, 0);
      frameBuffer = vector<vector<uint32_t>>();
      depthBuffer = vector<vector<float>>();
      for (int i = 0; i < canvasHeight; i++) {
        frameBuffer.push_back(vector<uint32_t>());
        depthBuffer.push_back(vector<float>());
        for (int j = 0; j < canvasWidth; j++) {
          frameBuffer[i].push_back(0);
          depthBuffer[i].push_back(0.0f);
        }
      }
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
      for (int i = 0; i < canvasHeight; i++) {
        for (int j = 0; j < canvasWidth; j++) {
          frameBuffer[i][j] = 0;
          depthBuffer[i][j] = 0.0f;
        }
      }
      if (isOrbiting) {
        vec3 pos = getPosition();
        vec3 newPos = vec3(getYRotationMatrix(3) * vec4(pos, 1));
        placement = glm::mat4(vec4(0, 0, 0, newPos.x), vec4(0, 0, 0, newPos.y), vec4(0, 0, 0, newPos.z), vec4(0, 0, 0 , 0)) + (placement + getTranslationMatrix(-pos));
      }
      if (isLooking) {
        vec3 forward = glm::normalize(vec3(lookTarget) - getPosition());
        vec3 right = glm::normalize(glm::cross(vec3(forward), vec3(0, 1, 0)));
        vec3 up = glm::normalize(glm::cross(vec3(forward), vec3(right)));
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
      return CanvasPoint(u, v, -1/vertexToCamera.z);
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

    RayTriangleIntersection getClosestIntersection(vec3 rayOrigin, vec3 ray, Scene scene) {
      vec3 closestSolution = vec3(-1e10, 0, 0);
      vec3 closestPoint;
      vec3 point;
      ModelTriangle solutionT;
      solutionT.colour = Colour(0, 0, 0);
      int solutionIndex = -1; // Represents No Solution
      int i = 0;
      // cout << "Triangles: " << scene.getModelTriangles().size();
      for (ModelTriangle triangle : *scene.getModelTriangles()) {
        vec3 e0 = vec3(triangle.vertices[1] - triangle.vertices[0]);
        vec3 e1 = vec3(triangle.vertices[2] - triangle.vertices[0]);
        vec3 SPVector = rayOrigin - vec3(triangle.vertices[0]);
        glm::mat3 DEMatrix(-ray, e0, e1);
        vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
        // cout << possibleSolution.x <<"\n";
        if (abs(possibleSolution.x) < abs(closestSolution.x)) {
          // cout << possibleSolution.x << " found, better than " << closestSolution.y << "\n";
          // cout << printVec(possibleSolution) << "\n";
          if (0.0f <= possibleSolution.y 
              && possibleSolution.y <= 1.0f 
              && 0.0f <= possibleSolution.z 
              && possibleSolution.z <= 1.0f 
              && possibleSolution.y + possibleSolution.z <= 1.0f 
              && possibleSolution.x < -0.001f) {
            closestSolution = possibleSolution;
            solutionT = triangle;
            point = vec3(solutionT.vertices[0])
                  + e0 * closestSolution.y
                  + e1 * closestSolution.z;
            closestPoint = point;
            solutionIndex = i;
          }
        }
        i++;
      }
      float distance = glm::length(rayOrigin - closestPoint);
      size_t index = solutionIndex;
      Material intersectionMaterial = *scene.getMaterial(index);
      if (intersectionMaterial.isTextured) {
        std::array<vec2, 3> ts = solutionT.texturePoints;
        vec2 e0 = ts[1] - ts[0];
        vec2 e1 = ts[2] - ts[0];
        vec2 point = ts[0] + e0 * closestSolution.y + e1 * closestSolution.z;
        uint32_t col = intersectionMaterial.getTexturePointColour(point);
        solutionT.colour = Colour((col >> 16) & 0xFF, (col >> 8) & 0xFF, col & 0xFF);
        // cout << point.x << " " << point.y << "\n";
      }
      RayTriangleIntersection intersection(closestPoint, distance, solutionT, index);
      return intersection;
    }

    RayTriangleIntersection getRaytracedPixelIntersection(int xPos, int yPos, Scene scene) {
      // Cast a ray
      vec3 rayDirection = getRayDirection(xPos, yPos);
      RayTriangleIntersection intersection = getClosestIntersection(getPosition(), rayDirection, scene);
      // If it intersects nothing, return early
      if (intersection.triangleIndex == -1) return intersection;

      // Everything must be at least 10% brightness
      float colourIntensity = 0.1f;
      // Iterate through every light in the scene
      for (vec3 lightSource : scene.lights) {
        // Determine if the light can see this point
        vec3 pointToLight = intersection.intersectionPoint - lightSource;
        RayTriangleIntersection lightIntersection = getClosestIntersection(intersection.intersectionPoint, pointToLight, scene);
        if (lightIntersection.triangleIndex != -1 
        // && (lightIntersection.distanceFromCamera > glm::length(pointToLight)) 
        && (intersection.triangleIndex != lightIntersection.triangleIndex)) {
          // if it can't, move on
          continue;
        }
        // determine brighness based on angle of incidence and distance
        float dotP = glm::dot(glm::normalize(pointToLight), intersection.intersectedTriangle.normal);
        if (dotP < 0.0f) dotP = 0.0f;
        if (dotP > 1.0f) dotP = 1.0f;
        float f = 5 / (glm::length(pointToLight) * glm::length(pointToLight));
        colourIntensity += f * dotP;
      }
      // cap the factor to 1
      if (colourIntensity > 1.0f) colourIntensity = 1.0f;
      Colour c = intersection.intersectedTriangle.colour;
      intersection.intersectedTriangle.colour = Colour(c.red*colourIntensity, c.green*colourIntensity, c.blue*colourIntensity);
      return intersection;
    }

    void raytraceSection(int x1, int x2, int y1, int y2, Scene *scene) {
      for (int i = y1; i < y2; i++) {
        for (int j = x1; j < x2; j++) {
          RayTriangleIntersection intersection = getRaytracedPixelIntersection(j, i, *scene);
			    ModelTriangle t = intersection.intersectedTriangle;
          if (intersection.triangleIndex == -1) continue;
          depthBuffer[i][j] = 1 / glm::length(getPosition() - intersection.intersectionPoint);
			    frameBuffer[i][j] = vec3ToColour(vec3(t.colour.red, t.colour.green, t.colour.blue), 255);
        }
      }
    }

    void rasterRender(Scene scene) {
      for (ModelTriangle triangle : *scene.getModelTriangles()) {
        CanvasPoint a = getCanvasIntersectionPoint(glm::vec3(triangle.vertices[0]));
        CanvasPoint b = getCanvasIntersectionPoint(glm::vec3(triangle.vertices[1]));
        CanvasPoint c = getCanvasIntersectionPoint(glm::vec3(triangle.vertices[2]));
        if (isInBounds(a, vec4(0, 0, canvasWidth, canvasHeight)) && isInBounds(b, vec4(0, 0, canvasWidth, canvasHeight)) && isInBounds(c, vec4(0, 0, canvasWidth, canvasHeight))) {
          CanvasTriangle canvasTriangle(a, b, c);
          filledTriangle(canvasTriangle, triangle.colour, frameBuffer, depthBuffer);
        }
      }
    }

    void wireframeRender(Scene scene) {
      for (ModelTriangle triangle : *scene.getModelTriangles()) {
        CanvasPoint a = getCanvasIntersectionPoint(glm::vec3(triangle.vertices[0]));
        CanvasPoint b = getCanvasIntersectionPoint(glm::vec3(triangle.vertices[1]));
        CanvasPoint c = getCanvasIntersectionPoint(glm::vec3(triangle.vertices[2]));
        if (isInBounds(a, vec4(0, 0, canvasWidth, canvasHeight)) && isInBounds(b, vec4(0, 0, canvasWidth, canvasHeight)) && isInBounds(c, vec4(0, 0, canvasWidth, canvasHeight))) {
          CanvasTriangle canvasTriangle(a, b, c);
          strokedTriangle(canvasTriangle, triangle.colour, frameBuffer, depthBuffer);
        }
      }
    }

    void raytraceRender(Scene scene) {
      vector<std::thread> threadVect;
      int slice_height = canvasHeight / threadCount;
      for (int i = 0; i < threadCount - 1; i++) {
        threadVect.push_back(std::thread(&Camera::raytraceSection, this, 0, canvasWidth, slice_height * i, slice_height * (i + 1), &scene));
      }
      threadVect.push_back(std::thread(&Camera::raytraceSection, this, 0, canvasWidth, slice_height * (threadCount - 1), canvasHeight, &scene));
      for (int i = 0; i < threadVect.size(); i++) {
        threadVect.at(i).join();
      }
    }

    void drawFancyBackground() {
      // Inspired by a Sebastian Lague video, I think.
      vec3 topLeft = glm::normalize(getRayDirection(0, 0));
      vec3 topRight = glm::normalize(getRayDirection(canvasWidth, 0));
      vec3 bottomLeft = glm::normalize(getRayDirection(0, canvasHeight));
      vec3 bottomRight = glm::normalize(getRayDirection(canvasWidth, canvasHeight));
      vector<vec3> leftEdge = interpolate(topLeft, bottomLeft, canvasHeight);
      vector<vec3> rightEdge = interpolate(topRight, bottomRight, canvasHeight);
      for (int i = 0; i < canvasHeight; i++) {
        vector<vec3> horizontalLine = interpolate(leftEdge[i], rightEdge[i], canvasWidth);
        for (int j = 0; j < canvasWidth; j++) {
          frameBuffer[i][j] = vec3ToColour(horizontalLine[j] * 128.0f + vec3(128.0f, 128.0f, 128.0f), 255);
        }
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
    void changeResolutionBy(int w, int h) {
      if (canvasWidth <= -w && canvasHeight <= -h) return;
      canvasWidth += w;
      canvasHeight += h;
      frameBuffer = vector<vector<uint32_t>>(canvasHeight);
      depthBuffer = vector<vector<float>>(canvasHeight);
      for (int i = 0; i < canvasHeight; i++) {
        frameBuffer[i] = vector<uint32_t>(canvasWidth);
        depthBuffer[i] = vector<float>(canvasWidth);
      }
    }

    private:
      bool isOrbiting;
      bool isLooking;
      vec4 lookTarget;
      float focalLength;
      float raytracingImagePlaneWidth;
      glm::mat4 placement;
};