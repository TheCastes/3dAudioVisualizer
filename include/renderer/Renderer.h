#pragma once

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <array>

class Renderer {
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer& copy) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&& move) = delete;
    Renderer& operator=(Renderer&&) noexcept = delete;

    bool init();
    void render(float currentAudioLevel) const;
    bool shouldClose() const;
private:
    GLuint loadShader(const char* shaderSource, int shaderType);
    void createBuffers(const std::array<float, 9>& triangleVertices);

    GLFWwindow* applicationWindow = nullptr;
    GLuint shaderProgram = 0;
    GLuint vertexArrayObject = 0;
    GLuint vertexBufferObject = 0;
    bool isInitialized = false;
};
