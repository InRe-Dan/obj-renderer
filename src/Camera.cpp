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
#include "Light.cpp"

using std::vector;
using glm::vec3;
using glm::vec2;

// Object to represent a camera in a scene.
class Camera : public Animateable {
  public:
    // How many threads to use for raytracing
    int threadCount;
    // Internal frame buffer width
    int canvasWidth;
    // Internal frame buffer height
    int canvasHeight;
    // Internal frame buffer
    vector<vector<uint32_t>> frameBuffer;
    // Internal depth buffer
    vector<vector<float>> depthBuffer;

    // Takes initializes a camera looking into -z
    Camera(vec2 resolution = vec2(640, 360), 
           vec3 position = vec3(0, 0, 10)
           ) {
      threadCount = 6;
      canvasHeight = resolution.y;
      canvasWidth = resolution.x;
      isLooking = false;
      imagePlaneWidth = 5.0f;
      focalLength = 3;
      placement = glm::mat4(1, 0, 0, position.x, 0, 1, 0, position.y, 0, 0, -1, position.z, 0, 0, 0, 0);
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

    // Look at the position of this vector. Will function if this value changes.
    void lookAt(vec3 *target) {
      lookTarget = target;
    }

    // Toggle looking
    void toggleLookAt() {
      isLooking = !isLooking;
    }

    // Clears frame buffers and looks towards the target if looking is on.
    void update() {
      for (int i = 0; i < canvasHeight; i++) {
        for (int j = 0; j < canvasWidth; j++) {
          frameBuffer[i][j] = 0;
          depthBuffer[i][j] = 0.0f;
        }
      }
      if (isLooking) {
        vec3 forward = glm::normalize(*lookTarget - getPosition());
        vec3 right = glm::normalize(glm::cross(vec3(forward), vec3(0, 1, 0)));
        vec3 up = - glm::normalize(glm::cross(vec3(forward), vec3(right)));
        vec3 pos = getPosition();
        placement = glm::mat4(vec4(right, pos.x), vec4(up, pos.y), vec4(forward, pos.z), vec4(0, 0, 0, 1));
      }
    }

    // Get placement matrix. Right side represents postiion.
    glm::mat4 getPlacement() {
      return placement;
    }

    // Get position of camera from placement matrix.
    vec3 getPosition() {
      return vec3(placement[0][3], placement[1][3], placement[2][3]);
    }

    // Set the position values in placement matrix.
    void setPosition(vec3 pos) {
      placement[0][3] = pos.x;
      placement[1][3] = pos.y;
      placement[2][3] = pos.z;
    }

    // Get an orientation matrix from placement
    glm::mat3 getOrientation() {
      return glm::mat3(placement);
    }

    // Project a point onto a CanvasPoint
    CanvasPoint getCanvasIntersectionPoint(vec3 vertexLocation) {
      // https://www.scratchapixel.com/lessons/3d-basic-rendering/computing-pixel-coordinates-of-3d-point/mathematics-computing-2d-coordinates-of-3d-points.html
      // All coordinates are relative to the camera!
      vec3 vertexToCamera = (vertexLocation - getPosition()) * getOrientation();
      float u = focalLength * (vertexToCamera.x/vertexToCamera.z) + imagePlaneWidth/2;
      float v = focalLength * (-vertexToCamera.y/vertexToCamera.z) + (canvasHeight * imagePlaneWidth / canvasWidth)/2;
      return CanvasPoint(glm::floor(canvasWidth * (u / imagePlaneWidth)), glm::floor(canvasHeight * v/ (canvasHeight * imagePlaneWidth / canvasWidth)), 1/vertexToCamera.z);
    }

    // Get ray direction for a given pixel on the internal frame buffer
    vec3 getRayDirection(int x, int y) {
      glm::mat3 o = getOrientation();
      vec3 right = glm::normalize(o[0]);
      vec3 up = glm::normalize(o[1]);
      vec3 forward = glm::normalize(o[2]);
      float pixelLength = imagePlaneWidth / canvasWidth;
      vec3 imagePlaneTopLeft = forward * focalLength + (up * (canvasHeight * 0.5f * pixelLength)) + (-right * (pixelLength * canvasWidth * 0.5f));
      return glm::normalize(imagePlaneTopLeft + float(x) * pixelLength * right + float(y) * -up * pixelLength);
    }

    // Find closest intersection in the direction of ray from rayOrigin.
    RayTriangleIntersection getClosestIntersection(vec3 rayOrigin, vec3 ray, Scene scene) {
      vec3 closestSolution = vec3(1e10, 0, 0);
      vec3 closestPoint;
      vec3 point;
      // Triangle with default nonsense values
      ModelTriangle solutionT;
      solutionT.colour = Colour(0, 0, 0);
      int solutionIndex = -1; // Represents No Solution
      int i = 0;
      for (ModelTriangle triangle : *scene.getModelTriangles()) {
        vec3 e0 = vec3(triangle.vertices[1] - triangle.vertices[0]);
        vec3 e1 = vec3(triangle.vertices[2] - triangle.vertices[0]);
        vec3 SPVector = rayOrigin - vec3(triangle.vertices[0]);
        glm::mat3 DEMatrix(-ray, e0, e1);
        vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
        if (possibleSolution.x < closestSolution.x) {
          // This final condition is for dealing with shadow acne
          if (0.0f <= possibleSolution.y 
              && possibleSolution.y <= 1.0f 
              && 0.0f <= possibleSolution.z 
              && possibleSolution.z <= 1.0f 
              && possibleSolution.y + possibleSolution.z <= 1.0f 
              && possibleSolution.x > 0.001f) {
            // Calculate intersection point in world space
            point = vec3(triangle.vertices[0])
                  + e0 * possibleSolution.y
                  + e1 * possibleSolution.z;
            closestSolution = possibleSolution;
            solutionT = triangle;
            closestPoint = point;
            solutionIndex = i;
          }
        }
        i++;
      }
      // If theres no itersection don't bother with any normal or texture calculations
      if (solutionIndex == -1) {
        RayTriangleIntersection intersection(closestPoint, closestSolution.x, solutionT, solutionIndex, solutionT.normal);
        return intersection;
      }

      // Calulate where the point lies in the texture space
      std::array<vec2, 3> ts = solutionT.texturePoints;
      vec2 e0 = ts[1] - ts[0];
      vec2 e1 = ts[2] - ts[0];
      vec2 texturePoint = ts[0] + e0 * closestSolution.y + e1 * closestSolution.z;
      if (solutionT.material->isTextured && scene.texturesEnabled) {
        uint32_t col = solutionT.material->getTexturePointColour(texturePoint);
        solutionT.colour = Colour((col >> 16) & 0xFF, (col >> 8) & 0xFF, col & 0xFF);
      }
      // Apply normal map if there is one available
      vec3 normal = solutionT.normal;
      if (solutionT.material->hasNormalMap && scene.normalMapsEnabled) {
        // Generating a matrix to rotate from tangent space to world space
        // Matrix represents rotation from "blue" in tangent space to the base triangle normal
        // https://en.wikipedia.org/wiki/Rotation_matrix
        vec3 tangentSpaceNormal = solutionT.material->getNormalMapVector(texturePoint);
        // Find axis and angle of rotation
        float angle = glm::acos(glm::dot(vec3(0, 0, 1), (normal)));
        vec3 axis = glm::cross(vec3(0, 0, 1), normal);
        // Precompute frequently needed values
        float c = cos(angle);
        float s = sin(angle);
        float t = 1 - c;
        // Create matrix
        glm::mat3 rotation = glm::mat3(c + axis.x * axis.x * t, axis.x * axis.y * t - axis.z * s, axis.x * axis.z * t + axis.y * s,
        axis.y * axis.x * t + axis.z * s, c + axis.y * axis.y * t, axis.y * axis.z * t - axis.x * s,
        axis.z * axis.x * t - axis.y * s, axis.z * axis.y * t + axis.x * s, c + axis.z * axis.z * t);
        // Apply to normalmap normal
        normal = tangentSpaceNormal * rotation;
      }
      // Apply smoothing logic if enabled and supported by obj
      if (solutionT.smoothingGroup != 0 && solutionT.hasVertexNormals && scene.smoothingEnabled) {
        if (scene.usingGouraudSmoothing) {
          // Unimplemented
        } else {
          vec3 e0n = solutionT.vertexNormals[1] - solutionT.vertexNormals[0];
          vec3 e1n = solutionT.vertexNormals[2] - solutionT.vertexNormals[0];
          vec3 newN = solutionT.vertexNormals[0] + e0n * closestSolution.y + e1n * closestSolution.z;
          normal = newN;
        }
      }
      RayTriangleIntersection intersection(closestPoint, closestSolution.x, solutionT, solutionIndex, normal);
      return intersection;
    }

    // Return the intersection information for a given pixel on the internal frame buffer. 
    RayTriangleIntersection getRaytracedPixelIntersection(int xPos, int yPos, Scene scene) {
      // Cast a ray
      vec3 rayDirection = getRayDirection(xPos, yPos);
      RayTriangleIntersection intersection = getClosestIntersection(getPosition(), rayDirection, scene);
      // If it intersects nothing, return early
      if (intersection.triangleIndex < 0) return intersection;
      if (!scene.lightingEnabled) return intersection;

      // Everything must be at least 10% brightness
      Colour c = intersection.intersectedTriangle.colour;
      vec3 originalColour = vec3(c.red, c.green, c.blue);
      vec3 ambient = 0.1f * originalColour;
      // Iterate through every light in the scene
      for (Light *lightSource : scene.getLights()) {
        // If light isn't on, skip
        if (!lightSource->state) continue;

        // Determine if the light can see this point
        vec3 lightToPoint = lightSource->pos - intersection.intersectionPoint;
        RayTriangleIntersection lightIntersection = getClosestIntersection(lightSource->pos, -lightToPoint, scene);
        if (lightIntersection.triangleIndex != -1 
        && (intersection.triangleIndex != lightIntersection.triangleIndex)) {
          // if it can't, skip
          continue;
        }

        // How much this light will affect brighness (values range from 0 - 255)
        vec3 lightImpact = vec3(0);

        // Using shading model from https://learnopengl.com/Lighting/Basic-Lighting
        // determine brightness based on angle of reflection:
        vec3 reflection = glm::normalize(lightToPoint - 2.0f * intersection.normal * (glm::dot(lightToPoint, intersection.normal)));
        float dotReflection = glm::dot(-rayDirection, reflection);
        if (dotReflection < 0.0f) dotReflection = 0.0f;
        if (dotReflection > 1.0f) dotReflection = 1.0f;
        float specular = glm::pow(dotReflection, intersection.intersectedTriangle.material->getSpecularExponent()) * 0.4f;

        // determine brighness based on angle of incidence
        float dotNormal = glm::dot(lightToPoint, intersection.normal);
        if (dotNormal < 0.0f) dotNormal = 0.0f;
        if (dotNormal > 1.0f) dotNormal = 1.0f;
        float diffuse = dotNormal * 0.5f;

        // Falloff based on distance from light
        float falloffFactor = lightSource->str / (glm::length(lightToPoint) * glm::length(lightToPoint));

        // Add to ambient light
        // lightImpact += originalColour * quantize(diffuse * falloffFactor + specular, 2) * (vec3(lightSource->r, lightSource->g, lightSource->b) / vec3(255));
        lightImpact += originalColour * (diffuse * falloffFactor + specular) * vec3(lightSource->r, lightSource->g, lightSource->b) / vec3(255);
        lightImpact = glm::min(lightImpact, vec3(255 * 0.9f));
        ambient += lightImpact;
      }
      // Cap light at 255
      ambient = glm::min(ambient, vec3(255 * 1.0f));
      intersection.intersectedTriangle.colour = Colour(ambient.r, ambient.g, ambient.b);
      return intersection;
    }

    // Raytrace on a particular section of the internal frame buffer
    // Intended to be passed into threads
    void raytraceSection(int x1, int x2, int y1, int y2, Scene *scene) {
      for (int i = y1; i < y2; i++) {
        for (int j = x1; j < x2; j++) {
          RayTriangleIntersection intersection = getRaytracedPixelIntersection(j, i, *scene);
			    ModelTriangle t = intersection.intersectedTriangle;
          if (intersection.triangleIndex < 0) continue;
          depthBuffer[i][j] = 1 / glm::length(getPosition() - intersection.intersectionPoint);
			    frameBuffer[i][j] = vec3ToColour(vec3(t.colour.red, t.colour.green, t.colour.blue), 255);
        }
      }
    }

    // Raster render on the internal frame buffer
    void rasterRender(Scene scene) {
      for (ModelTriangle triangle : *scene.getModelTriangles()) {
        CanvasPoint a = getCanvasIntersectionPoint(glm::vec3(triangle.vertices[0]));
        CanvasPoint b = getCanvasIntersectionPoint(glm::vec3(triangle.vertices[1]));
        CanvasPoint c = getCanvasIntersectionPoint(glm::vec3(triangle.vertices[2]));
        // Very unsophisticated clipping - remove if any vertex is outside view
        if (isInBounds(a, vec4(0, 0, canvasWidth, canvasHeight)) && isInBounds(b, vec4(0, 0, canvasWidth, canvasHeight)) && isInBounds(c, vec4(0, 0, canvasWidth, canvasHeight))) {
          CanvasTriangle canvasTriangle(a, b, c);
          filledTriangle(canvasTriangle, triangle.colour, frameBuffer, depthBuffer);
        }
      }
    }

    // Wireframe render on the internal frame buffer
    void wireframeRender(Scene scene) {
      for (ModelTriangle triangle : *scene.getModelTriangles()) {
        CanvasPoint a = getCanvasIntersectionPoint(glm::vec3(triangle.vertices[0]));
        CanvasPoint b = getCanvasIntersectionPoint(glm::vec3(triangle.vertices[1]));
        CanvasPoint c = getCanvasIntersectionPoint(glm::vec3(triangle.vertices[2]));
        // Very unsophisticated clipping - remove if any vertex is outside view
        if (isInBounds(a, vec4(0, 0, canvasWidth, canvasHeight)) && isInBounds(b, vec4(0, 0, canvasWidth, canvasHeight)) && isInBounds(c, vec4(0, 0, canvasWidth, canvasHeight))) {
          CanvasTriangle canvasTriangle(a, b, c);
          strokedTriangle(canvasTriangle, triangle.colour, frameBuffer, depthBuffer);
        }
      }
    }

    // Raytraced render on the internal frame buffer. Uses threadCount threads.
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
      drawLights(scene);
    }

    // Post-processing - render an approximation of where the light sources are in the scene
    void drawLights(Scene scene) {
      // If previews are disabled, pass
      if (!scene.lightPositionPreview) return;
      for (Light *light : scene.getLights()) {
        // Don't render lights that are too close
        if ((0.5 < glm::length(light->pos - getPosition()) && light->state)) {
          // Project to the focal plane
          CanvasPoint lightXY = getCanvasIntersectionPoint(light->pos);
          if (0 <= lightXY.x < canvasWidth && 0 <= lightXY.y < canvasHeight) {
            // Hack an illusion of depth by making circle size inversely proportional to depth
            circle(lightXY.x, lightXY.y, 0.1 * lightXY.depth, lightXY.depth, light->col, frameBuffer, depthBuffer);
          }
        }
      }
    }

    // Draw a backdrop to the frame buffer that visualizes each axis as the presence or absence of a colour channel.
    void drawFancyBackground() {
      /* 
      Inspired by a Sebastian Lague video, I think.
      Very much the same idea as one of the opening interpolation tasks. Just casts rays
      to each of the corners of the image plane to determine colours.
      */ 
      vec3 topLeft = glm::normalize(getRayDirection(0, 0));
      vec3 topRight = glm::normalize(getRayDirection(canvasWidth, 0));
      vec3 bottomLeft = glm::normalize(getRayDirection(0, canvasHeight));
      vec3 bottomRight = glm::normalize(getRayDirection(canvasWidth, canvasHeight));
      vector<vec3> leftEdge = interpolate(topLeft, bottomLeft, canvasHeight);
      vector<vec3> rightEdge = interpolate(topRight, bottomRight, canvasHeight);
      for (int i = 0; i < canvasHeight; i++) {
        vector<vec3> horizontalLine = interpolate(leftEdge[i], rightEdge[i], canvasWidth);
        for (int j = 0; j < canvasWidth; j++) {
          frameBuffer[i][j] = vec3ToColour(-horizontalLine[j] * 128.0f + vec3(128.0f, 128.0f, 128.0f), 255);
        }
      }
    }

    // Add a vector to camera position
    void moveBy(vec3 vect) {
      placement[0][3] += vect.x;
      placement[1][3] += vect.y;
      placement[2][3] += vect.z;
    }

    // Move in the "Up" direction relative to view
    void moveUp(float a) {
      moveBy(a * glm::normalize(vec3(placement[1])));
    }

    // Move in the negative "Up" direction relative to view
    void moveDown(float a) {
      moveUp(-a);
    }

    // Move in the "Right" direction relative to view
    void moveRight(float a) {
      moveBy(a * glm::normalize(vec3(placement[0])));
    }

    // Move in the negative "Right" direction relative to view
    void moveLeft(float a) {
      moveRight(-a);
    }

    // Move in the "Forward" direction relative to view
    void moveForward(float a) {
      moveBy(a * glm::normalize(vec3(placement[2])));
    }

    // Move in the negative "Forward" direction relative to view
    void moveBack(float a) {
      moveForward(-a);
    }

    // Tilt view up around x axis
    void lookUp(float degrees) {
      placement = getXRotationMatrix(-degrees) * placement;
    }

    // Tilt view down around x axis
    void lookDown(float degrees) {
      placement = getXRotationMatrix(degrees) * placement;
    }

    // Tilt view right around y axis
    void lookRight(float degrees) {
      placement = getYRotationMatrix(degrees) * placement;
    }

    // Tilt view left around y axis
    void lookLeft(float degrees) {
      placement = getYRotationMatrix(-degrees) * placement;
    }

    // Add some amount to focal length
    void changeF(float diff) {
      focalLength += diff;
    }

    // Get unit width of image plane
    float getImagePlaneWidth() {
      return imagePlaneWidth;
    }

    // Get focal length
    float getFocalLength() {
      return focalLength;
    }

    // Add w, h to resolution. Recreates frame and depth buffers.
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
      // Is the camera locked on to a target?
      bool isLooking;
      // Address of the position of the target
      vec3 *lookTarget;
      // Distance to image plane. Should be positive!
      float focalLength;
      // Unit length of image place. Height to be derived from aspect ratio.
      float imagePlaneWidth;
      // Placement matrix representing orientation and position
      glm::mat4 placement;
};