#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Mesh.h"

class SphereFieldMesh : public Mesh {
public:
    SphereFieldMesh(int n, int width, int depth) noexcept;
    ~SphereFieldMesh() noexcept override;

    void Draw() override;

private:
    struct Instance {
        glm::vec3 center;
        glm::vec2 uv;
    };

    std::vector<Instance> instances;
    GLuint instanceVBO = 0;

    void generateField(int n, int width, int depth);
    void setupMesh();
};