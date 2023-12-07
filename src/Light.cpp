#pragma once

#include "vecutil.cpp"

// Simple class to store light location and other attributes
class Light : public Animateable{
  public:
    Light(std::string n, vec3 position, float strength, Colour colour, bool startState, bool isSoft=false, float radius=0.05) {
      soft = isSoft;
      name = n;
      pos = position;
      str = strength;
      col = colour;
      r = col.red;
      g = col.green;
      b = col.blue;
      state = startState;
      this->radius = radius;
    }

    vec3 getPosition() {
      return pos;
    }

    void setPosition(vec3 position) {
      pos = position;
    }

    std::string name;
    vec3 pos;
    float str;
    int r;
    int g;
    int b;
    Colour col;
    bool state;
    bool soft;
    float radius;
};