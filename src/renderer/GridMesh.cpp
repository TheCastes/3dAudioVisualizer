#include <glad/glad.h>

#include "renderer/GridMesh.h"

GridMesh::GridMesh(int columns, int rows, int width, int depth) noexcept {
    generateGrid(columns, rows, width, depth);
    this->setupMesh();
}

void GridMesh::draw() {
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void GridMesh::generateGrid(int columns, int rows, int width, int depth) {
    vertices.reserve((columns + 1) * (rows + 1));
    for (int row = 0; row <=rows; ++row) {
        for (int col = 0; col <= columns; ++ col) {
            Vertex vertex;
            vertex.Position = {
                (col/(float)columns - 0.5f) * width,
                (row/(float)rows - 0.5f) * depth,
                0.0f
            };
            vertex.TexCoords = {
                col/(float)columns,
                row/(float)rows
            };
            vertices.push_back(vertex);
        }
    }

    indices.reserve(columns*rows*6);

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < columns; ++col) {
            GLuint topLeft = row * (columns + 1) + col;
            GLuint topRight = topLeft + 1;
            GLuint bottomLeft = (row + 1) * (columns + 1) + col;
            GLuint bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }

    }
}

void GridMesh::setupMesh() {
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