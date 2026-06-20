#include "../../include/flocking/BoidRenderer.h"
#include <iostream>
#include <cmath>

BoidRenderer::BoidRenderer() = default;

BoidRenderer::~BoidRenderer() {
    cleanup();
}

bool BoidRenderer::init() {
    shader = new Shader("../assets/shaders/boid.vert", "../assets/shaders/boid.frag");
    createBoidMesh();
    isInitialized = true;
    return true;
}

void BoidRenderer::createBoidMesh() {
    float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;

    float verts[][3] = {
        { 0,  1,  phi}, { 0, -1,  phi}, { 0,  1, -phi}, { 0, -1, -phi},
        { 1,  phi,  0}, {-1,  phi,  0}, { 1, -phi,  0}, {-1, -phi,  0},
        { phi, 0,  1}, { phi, 0, -1}, {-phi, 0,  1}, {-phi, 0, -1},
    };

    int faces[][3] = {
        {0,1,8},{0,8,4},{0,4,5},{0,5,10},{0,10,1},
        {1,6,8},{8,6,9},{8,9,4},{4,9,2},{4,2,5},
        {5,2,11},{5,11,10},{10,11,7},{10,7,1},{1,7,6},
        {3,6,7},{3,7,11},{3,11,2},{3,2,9},{3,9,6},
    };

    std::vector<float> vertices;
    for (auto& f : faces) {
        for (int j = 0; j < 3; ++j) {
            float x = verts[f[j]][0];
            float y = verts[f[j]][1];
            float z = verts[f[j]][2];
            float len = std::sqrt(x*x + y*y + z*z);
            vertices.push_back(x / len);
            vertices.push_back(y / len);
            vertices.push_back(z / len);
        }
    }

    vertexCount = 60;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
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

    glBindVertexArray(VAO);

    for (const Boid* boid : boids) {
        glm::mat4 modelMatrix = calculateModelMatrix(*boid);
        glUniformMatrix4fv(glGetUniformLocation(shader->Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    glBindVertexArray(0);
}

void BoidRenderer::cleanup() {
    if (!isInitialized) return;

    if (shader) {
        shader->Delete();
        delete shader;
        shader = nullptr;
    }

    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }

    isInitialized = false;
}
