#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Mesh.h"

class SphereFieldMesh : public Mesh {
public:
    SphereFieldMesh(int cellsPerSide, int width, int depth) noexcept;
    ~SphereFieldMesh() noexcept override;

    void draw() override;

private:
    struct Instance {
        glm::vec3 center;
        glm::vec2 uv;
    };

    std::vector<Instance> instances;
    GLuint instanceVBO = 0;

    void generateField(int cellsPerSide, int width, int depth);
    void setupMesh();
};