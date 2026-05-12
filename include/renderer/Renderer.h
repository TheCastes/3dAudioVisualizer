#pragma once

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

class Renderer {
public:
    static bool init();
    static void shutdown();
    static void render();
    static float getAudioLevel();
    static GLFWwindow* getWindow();
private:
    // bool buffersReady = false;

    static GLuint loadShader(const char* shaderSource, int shaderType);
    static void createBuffers(const std::array<float, 9>& triangleVertices);
};
