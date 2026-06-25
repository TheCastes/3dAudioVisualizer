#pragma once

#include <memory>
#include <string>
#include <vector>

#include "RenderMode.h"
#include "Shader.h"

class ShaderLibrary {
public:
    void add(const std::string& name, const GLchar* vertexPath, const GLchar* fragmentPath, RenderMode mode) {
        shaderNames.push_back(name);
        shaderModes.push_back(mode);
        shaders.push_back(std::make_unique<Shader>(vertexPath, fragmentPath));
    }

    Shader& getActiveShader() { return *shaders[activeShaderIndex]; }

    void setActive(int index) {
        if (index >= 0 && index < getCount())
            activeShaderIndex = index;
    }

    int getActiveIndex() const { return activeShaderIndex; }
    int getCount() const { return static_cast<int>(shaders.size()); }
    const std::vector<std::string>& getNames() const { return shaderNames; }
    const std::vector<RenderMode>& getModes() const { return shaderModes; }

    RenderMode getModeOf(int index) const { return shaderModes[index]; }

    int firstIndexForMode(RenderMode mode) const {
        for (int i = 0; i < getCount(); ++i)
            if (shaderModes[i] == mode) return i;
        return 0;
    }

    void clear() {
        shaders.clear();
        shaderNames.clear();
        shaderModes.clear();
        activeShaderIndex = 0;
    }

private:
    std::vector<std::string> shaderNames;
    std::vector<RenderMode> shaderModes;
    std::vector<std::unique_ptr<Shader>> shaders;
    int activeShaderIndex = 0;
};