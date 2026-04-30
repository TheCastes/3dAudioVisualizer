#include "../../include/renderer/Renderer.h"
#include "../../include/audio/AudioEngine.h"

#include <iostream>
#include <thread>
#include <cmath>

static GLFWwindow* g_window = nullptr;
static GLuint g_program = 0;
static GLuint g_vao = 0;
static GLuint g_vbo = 0;

bool Renderer::init() {
    if (!glfwInit()) { std::cerr << "GLFW init fallito\n"; return false; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    g_window = glfwCreateWindow(800, 600, "OpenGL 4.6 + JUCE 8 Audio", nullptr, nullptr);
    if (!g_window) { std::cerr << "Creazione finestra fallita\n"; return false; }
    glfwMakeContextCurrent(g_window);

    if (!gladLoadGL()) { std::cerr << "GLAD inizializzazione fallita\n"; return false; }
    std::cout << "GLAD OK | OpenGL: " << glGetString(GL_VERSION) << "\n";

    glm::vec3 v(3.0f, 4.0f, 0.0f);
    std::cout << "GLM OK | Length: " << glm::length(v) << "\n";

    const char* vs = "#version 460 core\nlayout(location=0) in vec3 p;\nvoid main() { gl_Position = vec4(p, 1.0); }\n";
    const char* fs = "#version 460 core\nout vec4 c; uniform float u_level;\nvoid main() { c = vec4(u_level, 0.3, 1.0 - u_level, 1.0); }\n";

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vs, nullptr); glCompileShader(vShader);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fs, nullptr); glCompileShader(fShader);

    g_program = glCreateProgram();
    glAttachShader(g_program, vShader); glAttachShader(g_program, fShader); glLinkProgram(g_program);
    glUseProgram(g_program);
    glDeleteShader(vShader); glDeleteShader(fShader);

    float verts[] = { -0.5f, -0.5f, 0.0f,  0.5f, -0.5f, 0.0f,  0.0f, 0.5f, 0.0f };
    glGenVertexArrays(1, &g_vao); glGenBuffers(1, &g_vbo);
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    std::cout << "✅ Rendering loop avviato...\n";
    return true;
}

void Renderer::shutdown() {
    glDeleteProgram(g_program);
    glDeleteBuffers(1, &g_vbo);
    glDeleteVertexArrays(1, &g_vao);
    glfwDestroyWindow(g_window);
    glfwTerminate();
}

void Renderer::render() {
    float level = getAudioLevel();
    glUniform1f(glGetUniformLocation(g_program, "u_level"), level);

    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(g_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glfwSwapBuffers(g_window);
}

float Renderer::getAudioLevel() {
    return AudioEngine::getAudioLevel().load(std::memory_order_relaxed);
}

GLFWwindow* Renderer::getWindow() {
    return g_window;
}
