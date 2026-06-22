#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Shader.h"

class ShaderLibrary {
public:
    void add(const std::string& name, const GLchar* vertexPath, const GLchar* fragmentPath) {
        shaderNames.push_back(name);
        shaders.push_back(std::make_unique<Shader>(vertexPath, fragmentPath));
    }

    Shader& active() { return *shaders[activeShaderIndex]; }

    void setActive(int index) {
        if (index >= 0 && index < count())
            activeShaderIndex = index;
    }

    int activeIndex() const { return activeShaderIndex; }
    int count() const { return static_cast<int>(shaders.size()); }
    const std::vector<std::string>& names() const { return shaderNames; }

    void clear() {
        shaders.clear();
        shaderNames.clear();
        activeShaderIndex = 0;
    }

private:
    std::vector<std::string> shaderNames;
    std::vector<std::unique_ptr<Shader>> shaders;
    int activeShaderIndex = 0;
};