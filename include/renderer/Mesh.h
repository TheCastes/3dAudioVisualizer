#pragma once

#include <vector>
#include <glm/glm.hpp>

// data structure for vertices
struct Vertex {
    // vertex coordinates
    glm::vec3 Position;
    // Normal
    glm::vec3 Normal;
    // Texture coordinates
    glm::vec2 TexCoords;
    // Tangent
    glm::vec3 Tangent;
    // Bitangent
    glm::vec3 Bitangent;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    GLuint VAO = 0;

    Mesh(int columns, int rows, int width, int depth) noexcept;
    ~Mesh() noexcept;

    Mesh(const Mesh& copy) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& move) noexcept;
    Mesh& operator=(Mesh&& move) noexcept;

    void Draw();


private:
    GLuint VBO, EBO;

    void generateGrid(int columns, int rows, int width, int depth);
    void setupMesh();
    void freeGPUresources();
};
