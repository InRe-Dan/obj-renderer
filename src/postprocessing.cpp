#include <vector>
#include <cstdint>

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
      return sum;
    }(kernelTemplate)),
    padding((size - 1) / 2) {}
    const int size;
    const float nFactor;
    const int padding;
    const vector<vector<float>> vect;
};


Kernel blurKernel = Kernel(vector<vector<float>>{
  vector<float>{1.0f, 1.0f, 1.0f},
  vector<float>{1.0f, 1.0f, 1.0f},
  vector<float>{1.0f, 1.0f, 1.0f}});

float getNormalizationFactor(vector<vector<float>> *kernel) {
  
}

void applyKernel(vector<vector<uint32_t>> &target, Kernel &k) {
  int targetW = target.at(0).size();
  int targetH = target.size();
  for (int i = k.padding; i < targetH - k.padding; i++) {
    for (int j = k.padding; j < targetW - k.padding; i++) {
      // write here lol
    }
  }
}