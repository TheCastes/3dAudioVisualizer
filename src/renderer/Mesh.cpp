#include <glad/glad.h>

#include "../../include/renderer/Mesh.h"

Mesh::~Mesh() noexcept {
    freeGPUresources();
}

void Mesh::freeGPUresources() noexcept {
    if (this->VAO) {
        glDeleteVertexArrays(1, &this->VAO);
        glDeleteBuffers(1, &this->VBO);
        glDeleteBuffers(1, &this->EBO);
    }
}