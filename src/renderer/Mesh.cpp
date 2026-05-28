#include <glad/glad.h>
#include "../../include/renderer/Mesh.h"

Mesh::Mesh(int columns, int rows, int width, int depth) noexcept {
    generateGrid(columns, rows, width, depth);
    this->setupMesh();
}

Mesh::~Mesh() noexcept {
    freeGPUresources();
}

Mesh::Mesh(Mesh&& move) noexcept
    : vertices(std::move(move.vertices)), indices(std::move(move.indices)), VAO(move.VAO), VBO(move.VBO), EBO(move.EBO) {
    move.VAO = 0;
}

Mesh& Mesh::operator=(Mesh&& move) noexcept {
    freeGPUresources();
    if (move.VAO) {
        this->vertices = std::move(move.vertices);
        this->indices = std::move(move.indices);
        this->VAO = move.VAO;
        this->VBO = move.VBO;
        this->EBO = move.EBO;
        move.VAO = 0;
    }
    else {
        this->VAO = 0;
    }
    return *this;
}

void Mesh::Draw() {
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::generateGrid(int columns, int rows, int width, int depth) {
    vertices.reserve((columns + 1) * (rows + 1));
    for (int row = 0; row <=rows; ++row) {
        for (int col = 0; col <= columns; ++ col) {
            Vertex v;
            v.Position = {
                (col/(float)columns - 0.5f) * width,
                (row/(float)rows - 0.5f) * depth,
                0.0f
            };
            v.TexCoords = {
                col/(float)columns,
                row/(float)rows
            };
            vertices.push_back(v);
        }
    }

    indices.reserve(columns*rows*6);

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < columns; ++col) {
            GLuint topleft = row * (columns + 1) + col;
            GLuint topright = topleft + 1;
            GLuint bottomleft = (row + 1) * (columns + 1) + col;
            GLuint bottomright = bottomleft + 1;

            indices.push_back(topleft);
            indices.push_back(bottomleft);
            indices.push_back(topright);
            indices.push_back(topright);
            indices.push_back(bottomleft);
            indices.push_back(bottomright);
        }

    }
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->EBO);

    glBindVertexArray(this->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
    // Texture Coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));
    // Tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Tangent));
    // Bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Bitangent));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::freeGPUresources() {
    if (this->VAO) {
        glDeleteVertexArrays(1, &this->VAO);
        glDeleteBuffers(1, &this->VBO);
        glDeleteBuffers(1, &this->EBO);
    }
}
