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
          uint32_t p = target.at(yo).at(xo);
          red += (p & 0x00FF0000) >> 16;
          blue += (p & 0x0000FF00) >> 8;
          green += (p & 0x000000FF);
        }
      }
      red /= k.nFactor;
      blue /= k.nFactor;
      green /= k.nFactor;
      uint32_t col = (red << 16) + (blue << 8) + green;
      result.at(i).at(j) = col;

    }
  }
  return result;
}