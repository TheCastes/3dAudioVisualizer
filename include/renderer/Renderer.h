#pragma once

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Mesh.h"
#include "Shader.h"
#include "Trackball.h"

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

    // void transform() const;

    static void glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    static void glfwCursorPosCallback(GLFWwindow *window, double x, double y);

    GLFWwindow* getWindow() const { return applicationWindow; }
private:
    //GLuint loadShader(const char* shaderSource, int shaderType);
    void createBuffers(const std::array<float, 9>& triangleVertices);

    const int screenWidth = 1280;
    const int screenHeight = 720;

    int viewportWidth = 0;
    int viewportHeight = 0;

    glm::mat4 projectionMatrix = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    GLFWwindow* applicationWindow = nullptr;
    Shader* shader = nullptr;
    Mesh* mesh = nullptr;
    Trackball trackball;
    GLuint vertexArrayObject = 0;
    GLuint vertexBufferObject = 0;
    bool isInitialized = false;
};
