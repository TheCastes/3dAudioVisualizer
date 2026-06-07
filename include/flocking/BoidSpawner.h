#pragma once

#include <vector>
#include <cstdlib>
#include <glm/glm.hpp>
#include "Boid.h"
#include "BoidConfig.h"

struct BoidSpawner {
    glm::vec3 origin = glm::vec3(0.0f);
    float radius = 10.0f;
    int count = 100;
    std::vector<Boid*> boids;

    void Spawn();
    void Update(const BoidConfig& config, float deltaTime);
    void Cleanup();

private:
    static glm::vec3 RandomPointInSphere(float radius);
};
