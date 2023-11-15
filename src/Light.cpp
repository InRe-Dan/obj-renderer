#pragma once

#include "vecutil.cpp"

class Light {
  public:
    Light(vec3 position, float strength, Colour col) {
      pos = position;
      str = strength;
      r = col.red;
      g = col.green;
      b = col.blue;
    }
    vec3 pos;
    float str;
    int r;
    int g;
    int b;
};