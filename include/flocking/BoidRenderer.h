#pragma once

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "Boid.h"
#include "BoidMesh.h"
#include "../renderer/Shader.h"

class BoidRenderer {
public:
    BoidRenderer();
    ~BoidRenderer();

    BoidRenderer(const BoidRenderer&) = delete;
    BoidRenderer& operator=(const BoidRenderer&) = delete;

    bool init();
    void render(const std::vector<Boid*>& boids, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    void cleanup();

private:
    BoidMesh* boidMesh = nullptr;
    Shader* shader = nullptr;
    bool isInitialized = false;

    glm::mat4 calculateModelMatrix(const Boid& boid) const;
};
