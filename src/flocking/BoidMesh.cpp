#include "../../include/flocking/BoidMesh.h"
#include <cmath>
#include <cstddef>

BoidMesh::BoidMesh() {
    generateIcosahedron();
    setupMesh();
}

BoidMesh::~BoidMesh() {
    freeGPUresources();
}

BoidMesh::BoidMesh(BoidMesh&& move) noexcept
    : vertices(std::move(move.vertices)), indices(std::move(move.indices)),
      VAO(move.VAO), VBO(move.VBO), EBO(move.EBO) {
    move.VAO = 0;
    move.VBO = 0;
    move.EBO = 0;
}

BoidMesh& BoidMesh::operator=(BoidMesh&& move) noexcept {
    freeGPUresources();
    if (move.VAO) {
        vertices = std::move(move.vertices);
        indices = std::move(move.indices);
        VAO = move.VAO;
        VBO = move.VBO;
        EBO = move.EBO;
        move.VAO = 0;
        move.VBO = 0;
        move.EBO = 0;
    }
    return *this;
}

void BoidMesh::generateIcosahedron() {
    const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;

    float raw[][3] = {
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

    vertices.resize(12);
    for (int i = 0; i < 12; ++i) {
        float x = raw[i][0], y = raw[i][1], z = raw[i][2];
        float len = std::sqrt(x * x + y * y + z * z);
        vertices[i].Position = glm::vec3(x / len, y / len, z / len);
        vertices[i].Normal = vertices[i].Position;
    }

    indices.reserve(60);
    for (auto& f : faces) {
        indices.push_back(f[0]);
        indices.push_back(f[1]);
        indices.push_back(f[2]);
    }
}

void BoidMesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void BoidMesh::draw() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void BoidMesh::freeGPUresources() {
    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        VAO = 0;
        VBO = 0;
        EBO = 0;
    }
}
