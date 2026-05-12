#include "../../include/renderer/Renderer.h"
#include "../../include/audio/AudioEngine.h"

#include <iostream>
#include <array>

static GLFWwindow* applicationWindow = nullptr;
static GLuint shaderProgram = 0;
static GLuint vertexArrayObject = 0;
static GLuint vertexBufferObject = 0;

bool Renderer::init() {

    if (!glfwInit()) { 
        std::cerr << "GLFW init fallito\n"; 
        return false; 
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    applicationWindow = glfwCreateWindow(800, 600, "3dAudioVisualizer", nullptr, nullptr);

    if (!applicationWindow) { 
        std::cerr << "Creazione finestra fallita\n"; 
        return false;
    }
    
    glfwMakeContextCurrent(applicationWindow);

    if (!gladLoadGL()) {
        std::cerr << "GLAD inizializzazione fallita\n"; 
        return false; 
    }
    std::cout << "GLAD OK | OpenGL: " << glGetString(GL_VERSION) << "\n";

    // TODO SHADER INLINE, TOCCA MODIFICARE
    const char* vertexShaderSource = "#version 460 core\nlayout(location=0) in vec3 p;\nvoid main() { gl_Position = vec4(p, 1.0); }\n"; //Vert
    const char* fragmentShaderSource = "#version 460 core\nout vec4 c; uniform float u_level;\nvoid main() { c = vec4(u_level, 0.3, 1.0 - u_level, 1.0); }\n"; //Frag

    const GLuint vertexShader = loadShader(vertexShaderSource, GL_VERTEX_SHADER);
    const GLuint fragmentShader = loadShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    const std::array<float, 9> triangleVertices = { -0.5f, -0.5f, 0.0f,  0.5f, -0.5f, 0.0f,  0.0f, 0.5f, 0.0f };

    createBuffers(triangleVertices);

    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    std::cout << "Rendering loop avviato...\n";
    return true;
}

void Renderer::createBuffers(const std::array<float, 9>& triangleVertices) {
    glGenVertexArrays(1, &vertexArrayObject);
    glGenBuffers(1, &vertexBufferObject);
    glBindVertexArray(vertexArrayObject);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, triangleVertices.size() * sizeof(float), triangleVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

GLuint Renderer::loadShader(const char* shaderSource, int shaderType) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr); 
    glCompileShader(shader);
    return shader;
}

void Renderer::shutdown() {
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &vertexBufferObject);
    glDeleteVertexArrays(1, &vertexArrayObject);
    glfwDestroyWindow(applicationWindow);
    glfwTerminate();
}

void Renderer::render() {
    float level = getAudioLevel();
    glUniform1f(glGetUniformLocation(shaderProgram, "u_level"), level);

    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(vertexArrayObject);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glfwSwapBuffers(applicationWindow);
}

float Renderer::getAudioLevel() {
    return AudioEngine::getAudioLevel().load(std::memory_order_relaxed);
}

GLFWwindow* Renderer::getWindow() {
    return applicationWindow;
}
