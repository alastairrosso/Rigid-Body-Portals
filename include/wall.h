#include <glm/glm.hpp>

using namespace glm;

struct Wall {
    vec2 pt1;
    vec2 pt2;
    float A;
    float B;
    float sqAB;
    vec2 unit_tan;
    vec2 unit_nor;
    mat4 A_a2g;
    mat4 A_g2a;

    Wall() {}
    Wall(vec2 pt_1, vec2 pt_2) : A(pt_1.y - pt_2.y), B(pt_2.x - pt_1.x) {
        pt1 = pt_1;
        pt2 = pt_2;

        vec2 displ = pt2 - pt1;
        unit_tan = normalize(displ);
        unit_nor = vec2(-unit_tan.y, unit_tan.x);

        A_a2g = mat4(
            unit_tan.x, unit_tan.y, 0.0f, 0.0f,
            unit_nor.x, unit_nor.y, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
        A_g2a = inverse(A_a2g);
    }

    float dist(float x, float y) {
        vec2 displ_b1 = vec2(x, y) - pt1;
        vec2 displ_12 = pt2 - pt1;
        float v_mag = glm::length(displ_12);
        float t_min = glm::dot(displ_b1, displ_12) / (v_mag*v_mag);
        t_min = std::clamp(t_min, 0.0f, 1.0f);

        vec2 pt_closest = pt1 + t_min*displ_12;
        vec2 displ = vec2(x, y) - pt_closest;
        return glm::length(displ);
        // return sqrt(displ.x*displ.x + displ.y*displ.y);
    }
};
