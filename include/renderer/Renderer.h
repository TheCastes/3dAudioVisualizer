#pragma once

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Renderer {
public:
    static bool init();
    static void shutdown();
    static void render();
    static float getAudioLevel();
    static GLFWwindow* getWindow();
private:
    bool buffersReady = false;
    static GLuint loadShader(const char* s, int type);
    static void createBuffers(float verts[]);
};