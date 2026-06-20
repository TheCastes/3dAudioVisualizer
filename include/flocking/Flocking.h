#pragma once

#include "BoidConfig.h"
#include "BoidSpawner.h"
#include "BoidRenderer.h"

class Flocking {
public:
    bool init();
    void spawn();
    bool isSpawned() const;
    void update(float currentAudioLevel, float deltaTime, bool isPlaying);
    void render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    void cleanup();

private:
    BoidConfig boidConfig;
    BoidSpawner boidSpawner;
    BoidRenderer boidRenderer;
    bool spawned = false;
};
