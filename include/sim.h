#include <iostream>
#include <glm/glm.hpp>

#include "wall.h"

using namespace glm;

class Sim {
private:
    float dt;
    Wall walls[6];
    float ball_radius;
    vec2 ball_pos;
    vec2 ball_vel;
public:
    Sim(float);
    void update();
    vec2 getBallPos();
};
