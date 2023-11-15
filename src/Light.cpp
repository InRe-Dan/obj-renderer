#pragma once

#include "vecutil.cpp"

class Light {
  public:
    Light(std::string n, vec3 position, float strength, Colour col, bool startState) {
      name = n;
      pos = position;
      str = strength;
      r = col.red;
      g = col.green;
      b = col.blue;
      state = startState;
    }
    std::string name;
    vec3 pos;
    float str;
    int r;
    int g;
    int b;
    bool state;
};