#pragma once

#include <vector>
#include <cstdint>
#include <stdio.h>


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
    const float nFactor;
    const int padding;
};


Kernel boxBlurKernel = Kernel(vector<vector<float>>{
  vector<float>{1.0f, 1.0f, 1.0f},
  vector<float>{1.0f, 1.0f, 1.0f},
  vector<float>{1.0f, 1.0f, 1.0f}});

Kernel sharpenKernel = Kernel(vector<vector<float>>{
  vector<float>{0.0f, -1.0f, 0.0f},
  vector<float>{-1.0f, 5.0f, -1.0f},
  vector<float>{0.0f, -1.0f, 0.0f}});

Kernel edgeDetectionKernel = Kernel(vector<vector<float>>{
  vector<float>{-1.0f, -1.0f, -1.0f},
  vector<float>{-1.0f, 8.0f, -1.0f},
  vector<float>{-1.0f, -1.0f, -1.0f}});

vector<vector<uint32_t>> applyKernel(vector<vector<uint32_t>> &target, Kernel &k) {
  int targetW = target.at(0).size();
  int targetH = target.size();
  static vector<vector<uint32_t>> result(target);
  for (int i = k.padding; i < targetH - k.padding; i++) {
    for (int j = k.padding; j < targetW - k.padding; j++) {
      uint32_t red = 0, blue = 0, green = 0;
      for (int y = 0; y < k.size; y++) {
        for (int x = 0; x < k.size; x++) {
          int yo = i + (y - k.padding);
          int xo = j + (x - k.padding);
          uint32_t p = target[yo][xo];
          red += (p & 0x00FF0000) >> 16;
          blue += (p & 0x0000FF00) >> 8;
          green += (p & 0x000000FF);
        }
      }
      red /= k.nFactor;
      blue /= k.nFactor;
      green /= k.nFactor;
      uint32_t col = (red << 16) + (blue << 8) + green;
      result[i][j] = col;

    }
  }
  return result;
}

vector<vector<uint32_t>> blackAndWhite(vector<vector<uint32_t>> &target) {
  int targetW = target.at(0).size();
  int targetH = target.size();
  for (int i = 0; i < targetH; i++) {
    for (int j = 0; j < targetW; j++){
      uint32_t col = target[i][j];
      // These weights are based on approximations of how the eye percieves different channels.
      // Source: e2eml.school - Course 137
      uint32_t sum = ((col >> 16) & 0x00FF0000) * 0.299 + ((col >> 8) & 0x0000FF00) * 0.587 + (col & 0x000000FF) * 0.114;
      sum = sum > 255? 255 : sum;
      uint32_t repacked = (sum << 16) + (sum << 8) + sum;
      target[i][j] = repacked;

    }
  }
  return target;
}

void arbitraryUpscale(vector<vector<uint32_t>> source, vector<vector<uint32_t>>& target) {
  assert(source.size() <= target.size());
  float ratio = target.size() / source.size();
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