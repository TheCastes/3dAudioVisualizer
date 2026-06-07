#pragma once

// i hate this
struct BoidConfig {
    float fov = 2.0f;
    float speed = 10.0f;
    float separationWeight = 1.0f;
    float cohesionWeight = 1.0f;
    float alignWeight = 1.0f;
};
