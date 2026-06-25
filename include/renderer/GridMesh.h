#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Mesh.h"

class GridMesh : public Mesh {
public:
    GridMesh(int columns, int rows, int width, int depth) noexcept;

    void draw() override;

private:
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

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    void generateGrid(int columns, int rows, int width, int depth);
    void setupMesh();
};