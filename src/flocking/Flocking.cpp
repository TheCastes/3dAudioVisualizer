#include "../../include/flocking/Flocking.h"
#include <iostream>

bool Flocking::init() {
    if (!boidRenderer.init()) {
        std::cerr << "BoidRenderer init fallito\n";
        return false;
    }

    // TODO expose config params
    boidConfig.fov = 2.0f;
    boidConfig.speed = 1.0f;
    boidConfig.separationWeight = 3.0f;
    boidConfig.cohesionWeight = 2.0f;
    boidConfig.alignWeight = 1.0f;

    boidSpawner.origin = glm::vec3(0.0f, 0.0f, -40.0f);
    boidSpawner.count = 500;
    boidSpawner.radius = 10.0f;
    boidSpawner.Spawn();

    // TODO ? containment box

    return true;
}

void Flocking::update(float currentAudioLevel, float deltaTime) {
    boidConfig.speed = 5.0f + currentAudioLevel * 15.0f;
    boidConfig.fov = 1.5f + currentAudioLevel * 3.0f;
    boidConfig.separationWeight = 0.8f + currentAudioLevel * 2.8f;
    boidConfig.cohesionWeight = 1.0f - currentAudioLevel * 0.8f;
    boidConfig.alignWeight = 0.8f + currentAudioLevel * 0.7f;

    boidSpawner.Update(boidConfig, deltaTime);
}

void Flocking::render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    boidRenderer.render(boidSpawner.boids, viewMatrix, projectionMatrix);
}

void Flocking::cleanup() {
    boidSpawner.Cleanup();
    boidRenderer.cleanup();
}
