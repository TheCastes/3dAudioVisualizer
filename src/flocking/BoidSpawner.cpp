#include "../../include/flocking/BoidSpawner.h"
#include <cstdlib>

void BoidSpawner::Spawn() {
    for (int i = 0; i < count; ++i) {
        Boid* boid = new Boid();
        boid->position = origin + RandomPointInSphere(radius);

        glm::vec3 lookTarget = origin + RandomPointInSphere(radius);
        boid->direction = glm::normalize(lookTarget - boid->position);

        boids.push_back(boid);
    }
}

void BoidSpawner::Update(const BoidConfig& config, float deltaTime) {
    for (Boid* boid : boids) {
        boid->align.weight = config.alignWeight;
        boid->cohesion.weight = config.cohesionWeight;
        boid->separation.weight = config.separationWeight;
        boid->Update(config, deltaTime, boids);
    }
}

void BoidSpawner::Cleanup() {
    for (Boid* boid : boids) {
        delete boid;
    }
    boids.clear();
}

glm::vec3 BoidSpawner::RandomPointInSphere(float radius) {
    float x = (float)std::rand() / RAND_MAX * 2.0f - 1.0f;
    float y = (float)std::rand() / RAND_MAX * 2.0f - 1.0f;
    float z = (float)std::rand() / RAND_MAX * 2.0f - 1.0f;
    glm::vec3 p(x, y, z);
    float len = glm::length(p);
    if (len > 1.0f) {
        p = glm::normalize(p);
    }
    return p * radius;
}
