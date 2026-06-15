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
    void swapBuffers() const;
    bool shouldClose() const;

    static void glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    static void glfwCursorPosCallback(GLFWwindow *window, double x, double y);

    static void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    GLFWwindow* getWindow() const { return applicationWindow; }
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
private:
    const int screenWidth = 1280;
    const int screenHeight = 720;

    static constexpr float renderFractionW = 0.7f;
    static constexpr float renderFractionH = 0.7f;

    int viewportWidth = 0;
    int viewportHeight = 0;

    bool cursorToSubViewport(double x, double y, float& localX, float& localY, int& subW, int& subH) const;

    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    GLFWwindow* applicationWindow = nullptr;
    Shader* shader = nullptr;
    Mesh* mesh = nullptr;
    Trackball trackball;
    GLuint vertexArrayObject = 0;
    GLuint vertexBufferObject = 0;
    bool isInitialized = false;
};
