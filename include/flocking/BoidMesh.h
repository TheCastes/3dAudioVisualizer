#pragma once

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <vector>

#include "../renderer/Mesh.h"

class BoidMesh {
public:
    BoidMesh();
    ~BoidMesh();

    BoidMesh(const BoidMesh& copy) = delete;
    BoidMesh& operator=(const BoidMesh&) = delete;
    BoidMesh(BoidMesh&& move) noexcept;
    BoidMesh& operator=(BoidMesh&& move) noexcept;

    void draw();

private:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    void generateIcosahedron();
    void setupMesh();
    void freeGPUresources();
};
