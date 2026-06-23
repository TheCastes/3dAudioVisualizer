#pragma once

#include <glm/glm.hpp>

class Mesh {
public:
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    Mesh() = default;
    virtual ~Mesh() noexcept;

    Mesh(const Mesh& copy) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& move) = delete;
    Mesh& operator=(Mesh&& move) = delete;

    virtual void Draw() = 0;

protected:
    GLuint VAO = 0, VBO = 0, EBO = 0;

    void freeGPUresources() noexcept;
};