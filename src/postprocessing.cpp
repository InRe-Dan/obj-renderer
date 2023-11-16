#pragma once

#include <vector>
#include <cstdint>
#include <stdio.h>
#include <glm/glm.hpp>

using glm::vec3;
using glm::vec2;


using std::vector;

class Kernel {
  public:
    Kernel(vector<vector<float>> kernelTemplate) : 
    vect(kernelTemplate), 
    size(kernelTemplate.size()), 
    nFactor([](vector<vector<float>> kernelTemplate) {
      float sum = 0.0f;
      for (vector<float> column : kernelTemplate) {
        for (float num : column) {
          sum += num;
        }
      }
      // Return sum only if non-zero - else return 1,
      return sum? sum : 1;
    }(kernelTemplate)),
    padding((size - 1) / 2) {}
    const vector<vector<float>> vect;
    const int size;
    const int nFactor;
    const int padding;
};


Kernel boxBlurKernel = Kernel(vector<vector<float>>{
  vector<float>{1.0f, 1.0f, 1.0f},
  vector<float>{1.0f, 1.0f, 1.0f},
  vector<float>{1.0f, 1.0f, 1.0f}});

Kernel gaussian3x3 = Kernel(vector<vector<float>>{
  vector<float>{1.0f, 2.0f, 1.0f},
  vector<float>{2.0f, 4.0f, 2.0f},
  vector<float>{1.0f, 2.0f, 1.0f}});

Kernel sharpenKernel = Kernel(vector<vector<float>>{
  vector<float>{0.0f, -1.0f, 0.0f},
  vector<float>{-1.0f, 5.0f, -1.0f},
  vector<float>{0.0f, -1.0f, 0.0f}});

Kernel edgeDetectionKernelH = Kernel(vector<vector<float>>{
  vector<float>{1.0f, 2.0f, 1.0f},
  vector<float>{0.0f, 0.0f, 0.0f},
  vector<float>{-1.0f, -2.0f, -1.0f}});
Kernel edgeDetectionKernelV = Kernel(vector<vector<float>>{
  vector<float>{-1.0f, 0.0f, 1.0f},
  vector<float>{-2.0f, 0.0f, 2.0f},
  vector<float>{-1.0f, 0.0f, 1.0f}});

  Kernel edgeDetectionFull = Kernel(vector<vector<float>>{
    vector<float>{-1.0f, -1.0f, -1.0f},
    vector<float>{-1.0f, 8.0f, -1.0f},
    vector<float>{-1.0f, -1.0f, -1.0f}});

// Does not apply to the edges of the target.
vector<vector<uint32_t>> applyKernel(vector<vector<uint32_t>> &target, Kernel &k) {
  int targetW = target.at(0).size();
  int targetH = target.size();
  vector<vector<uint32_t>> result(target);
  for (int i = k.padding; i < targetH - k.padding; i++) {
    for (int j = k.padding; j < targetW - k.padding; j++) {
      float red = 0, blue = 0, green = 0;
      for (int y = 0; y < k.size; y++) {
        for (int x = 0; x < k.size; x++) {
          int yo = i + (y - k.padding);
          int xo = j + (x - k.padding);
          uint32_t p = target[yo][xo];
          red += ((p & 0x00FF0000) >> 16) * k.vect[y][x];
          green += ((p & 0x0000FF00) >> 8) * k.vect[y][x];
          blue += ((p & 0x000000FF)) * k.vect[y][x];
        }
      }
      int r = int(red / k.nFactor);
      int g = int(green / k.nFactor);
      int b = int(blue / k.nFactor);
      r = (r > 255)? 255 : r;
      g = (g > 255)? 255 : g;
      b = (b > 255)? 255 : b;
      uint32_t col = (r << 16) + (g << 8) + b;
      result[i][j] = col;

    }
  }
  return result;
}

void blackAndWhite(vector<vector<uint32_t>> &target) {
  int targetW = target.at(0).size();
  int targetH = target.size();
  for (int i = 0; i < targetH; i++) {
    for (int j = 0; j < targetW; j++){
      uint32_t col = target[i][j];
      // These weights are based on approximations of how the eye percieves different channels.
      // Source: e2eml.school - Course 137
      uint32_t sum = ((col >> 16) & 0x000000FF) * 0.299 + ((col >> 8) & 0x000000FF) * 0.587 + (col & 0x000000FF) * 0.114;
      sum = sum > 255? 255 : sum;
      uint32_t repacked = (sum << 16) + (sum << 8) + sum;
      target[i][j] = repacked;

    }
  }
}

// Assumes image to be monochromatic. Applies thresholding to all pixels.
void threshold(vector<vector<uint32_t>> &target, vec3 thresholds) {
  int targetW = target.at(0).size();
  int targetH = target.size();
  for (int i = 0; i < targetH; i++) {
    for (int j = 0; j < targetW; j++){
      uint32_t col = target[i][j];
      uint8_t r = (((col >> 16) & 0x000000FF) > thresholds.x)? 255 : 0;
      uint8_t g = (((col >> 8) & 0x000000FF) > thresholds.y)? 255 : 0;
      uint8_t b = ((col & 0x000000FF) > thresholds.z)? 255 : 0;
      uint32_t repacked = (r << 16) + (g << 8) + b;
      target[i][j] = repacked;

    }
  }
}

// Assumes images are monochromatic. Find the hypotenuse of two images, normalize them to 0-255 and 
// write them into the target.
void hypot(vector<vector<uint32_t>> &target, vector<vector<uint32_t>> &one, vector<vector<uint32_t>> &two) {
  int targetW = target.at(0).size();
  int targetH = target.size();
  for (int i = 0; i < targetH; i++) {
    for (int j = 0; j < targetW; j++){
      uint32_t col1 = one[i][j];
      uint32_t col2 = two[i][j];
      uint32_t r1 = ((col1 >> 16) & 0x000000FF);
      uint32_t g1 = ((col1 >> 8) & 0x000000FF);
      uint32_t b1 = (col1 & 0x000000FF);
      uint32_t r2 = ((col2 >> 16) & 0x000000FF);
      uint32_t g2 = ((col2 >> 8) & 0x000000FF);
      uint32_t b2 = (col2 & 0x000000FF);
      float maxLength = glm::length(vec2(255, 255));
      uint8_t r = glm::min(255, int(255 * glm::length(vec2(r1, r2)) / maxLength));
      uint8_t g = glm::min(255, int(255 * glm::length(vec2(g1, g2)) / maxLength));
      uint8_t b = glm::min(255, int(255 * glm::length(vec2(b1, b2)) / maxLength));
      uint32_t repacked = (r << 16) + (g << 8) + b;
      target[i][j] = repacked;
    }
  }
}

// Upscales any frame buffer into an equal or larger one.
void arbitraryUpscale(vector<vector<uint32_t>> source, vector<vector<uint32_t>>& target) {
  assert(source.size() <= target.size());
  // float ratio = target.size() / source.size();
  int nearestx = 0;
  int nearesty = 0;
  int sWidth = source.at(0).size();
  int sHeight = source.size();
  int tWidth = target.at(0).size();
  int tHeight = target.size();
  for (int i = 0; i < target.size() ; i++) {
    for (int j = 0; j < target.at(0).size(); j++) {
      nearestx = glm::round((float(j)/tWidth) * sWidth);
      nearesty = glm::round((float(i)/tHeight) * sHeight);
      if (nearestx > sWidth - 1) nearestx = sWidth - 1;
      if (nearestx < 0) nearestx = 0;
      if (nearesty > sHeight - 1) nearesty = sHeight - 1;
      if (nearesty < 0) nearesty = 0;
      target[i][j] = source[nearesty][nearestx];
    }
  }
}

// This is a work in progress - nearest neighbour seems sufficient.
void bilinearUpscale(vector<vector<uint32_t>> source, vector<vector<uint32_t>>& target) {
  assert(source.size() <= target.size());
  int sWidth = source.at(0).size();
  int sHeight = source.size();
  int tWidth = target.at(0).size();
  int tHeight = target.size();
  for (int i = 0; i < target.size() ; i++) {
    for (int j = 0; j < target.at(0).size(); j++) {
      int lowerx = glm::floor((float(j)/tWidth) * sWidth);
      int lowery = glm::floor((float(i)/tHeight) * sHeight);
      if (lowerx < 0) lowerx = 0;
      if (lowery < 0) lowery = 0;
      int higherx = lowerx + 1;
      int highery = lowery + 1;
      if (higherx > sWidth - 1) higherx = sWidth - 1;
      if (highery > sHeight - 1) highery = sHeight - 1;
      uint8_t* argb1 = (uint8_t*) &(source[lowery][lowerx]);
      uint8_t* argb2 = (uint8_t*) &(source[highery][higherx]);
      for (int channel = 1; channel < 4; channel++) {
        // Values here should be weighted and both dimensions (not just diagonal) should be used
        argb1[channel] = argb1[channel] + argb2[channel] > 1;
      }
      target[i][j] = source[lowery][lowerx];
    }
  }
}