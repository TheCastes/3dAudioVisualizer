#include <glad/glad.h>

#include "../../include/renderer/SphereFieldMesh.h"

namespace {
    constexpr float quadCorners[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
        -0.5f,  0.5f,
         0.5f,  0.5f,
    };
}

SphereFieldMesh::SphereFieldMesh(int n, int width, int depth) noexcept {
    generateField(n, width, depth);
    this->setupMesh();
}

SphereFieldMesh::~SphereFieldMesh() noexcept {
    if (this->instanceVBO)
        glDeleteBuffers(1, &this->instanceVBO);
}

void SphereFieldMesh::Draw() {
    glBindVertexArray(this->VAO);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, this->instances.size());
    glBindVertexArray(0);
}

void SphereFieldMesh::generateField(int n, int width, int depth) {
    instances.reserve(n * n);
    for (int row = 0; row < n; ++row) {
        for (int col = 0; col < n; ++col) {
            Instance inst;
            float u = (col + 0.5f) / n;
            float v = (row + 0.5f) / n;
            inst.center = {
                (u - 0.5f) * width,
                (v - 0.5f) * depth,
                0.0f
            };
            inst.uv = { u, v };
            instances.push_back(inst);
        }
    }
}

void SphereFieldMesh::setupMesh() {
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->instanceVBO);

    glBindVertexArray(this->VAO);

    // per-vertex: unit quad corners (location 0, divisor 0)
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadCorners), quadCorners, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (GLvoid*)0);

    // per-instance: cell center + uv (locations 1, 2, divisor 1)
    glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, this->instances.size() * sizeof(Instance), &this->instances[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Instance), (GLvoid*)offsetof(Instance, center));
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Instance), (GLvoid*)offsetof(Instance, uv));
    glVertexAttribDivisor(2, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}