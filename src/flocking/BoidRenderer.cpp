#include "../../include/flocking/BoidRenderer.h"
#include <iostream>

BoidRenderer::BoidRenderer() = default;

BoidRenderer::~BoidRenderer() {
    cleanup();
}

bool BoidRenderer::init() {
    boidMesh = new BoidMesh();
    shader = new Shader("../assets/shaders/boid.vert", "../assets/shaders/boid.frag");
    isInitialized = true;
    return true;
}

glm::mat4 BoidRenderer::calculateModelMatrix(const Boid& boid) const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, boid.position);
    model = glm::scale(model, glm::vec3(0.1f));
    return model;
}

void BoidRenderer::render(const std::vector<Boid*>& boids, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    if (!isInitialized) return;

    shader->Use();

    glUniformMatrix4fv(glGetUniformLocation(shader->Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader->Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    for (const Boid* boid : boids) {
        glm::mat4 modelMatrix = calculateModelMatrix(*boid);
        glUniformMatrix4fv(glGetUniformLocation(shader->Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        boidMesh->draw();
    }
}

void BoidRenderer::cleanup() {
    if (!isInitialized) return;

    if (boidMesh) {
        delete boidMesh;
        boidMesh = nullptr;
    }

    if (shader) {
        shader->Delete();
        delete shader;
        shader = nullptr;
    }

    isInitialized = false;
}
