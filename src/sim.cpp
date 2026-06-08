#include "sim.h"

Sim::Sim(float _dt) : dt(_dt) {
    Wall w0(vec2(-15.0f,  11.0f), vec2(-15.0f,   2.0f));
    Wall w1(vec2(-15.0f,   2.0f), vec2(-10.0f,  -2.0f));
    Wall w2(vec2(-10.0f,  -2.0f), vec2(-10.0f, -11.0f));
    Wall w3(vec2(-10.0f, -11.0f), vec2( 15.0f, -11.0f));
    Wall w4(vec2( 15.0f, -11.0f), vec2( 15.0f,  11.0f));
    Wall w5(vec2( 15.0f,  11.0f), vec2(-15.0f,  11.0f));
    walls[0] = w0; // upper left
    walls[1] = w1; // ramp
    walls[2] = w2; // lower left
    walls[3] = w3; // floor
    walls[4] = w4; // right
    walls[5] = w5; // ceiling

    ball_radius = 0.5f;
    ball_pos = vec2(-12.5f, 5.0f);
    ball_vel = vec2(-0.5f, -15.0f);
}

void Sim::update() {
    ball_pos = ball_pos + dt*ball_vel;
    ball_vel = ball_vel + dt*vec2(0.0f, -9.8f);

    for (Wall w : walls) {
        if (w.dist(ball_pos.x, ball_pos.y) < ball_radius) {
            vec2 ball_vel_s = w.A_g2a * vec4(ball_vel, 0.0f, 1.0f);
            ball_vel_s = 0.85f * vec2(ball_vel_s.x, abs(ball_vel_s.y));
            ball_vel = w.A_a2g * vec4(ball_vel_s, 0.0f, 1.0f);
        }
    }
}

vec2 Sim::getBallPos() {
    return ball_pos;
}
