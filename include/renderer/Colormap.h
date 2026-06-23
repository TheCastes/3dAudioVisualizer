#pragma once

#include <array>
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct Colormap {
    std::string name;
    std::array<float, 5> positions;
    std::array<glm::vec3, 5> colors;
};

inline std::vector<Colormap> defaultColormaps() {
    return {
        {
            "Thermal",
            { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f },
            {
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 1.0f),
                glm::vec3(0.0f, 1.0f, 1.0f),
                glm::vec3(1.0f, 1.0f, 0.0f),
                glm::vec3(1.0f, 0.0f, 0.0f)
            }
        },
        {
            "Grayscale",
            { 0.0f, 1.0f },
            {
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(1.0f, 1.0f, 1.0f)
            }
        },
        {
            "Dracula",
            { 0.0f, 0.3f, 0.6f, 0.9f, 1.0f },
            {
                glm::vec3(0.107f, 0.115f, 0.162f),
                glm::vec3(0.334f, 0.397f, 0.593f),
                glm::vec3(0.691f, 0.526f, 0.926f),
                glm::vec3(0.949f, 0.425f, 0.726f),
                glm::vec3(0.895f, 0.930f, 0.499f)
            }
        }
    };
}
