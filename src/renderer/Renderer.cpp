#include "../../include/renderer/Renderer.h"
#include "../../include/audio/AudioEngine.h"

#include <iostream>
#include <thread>
#include <cmath>

static GLFWwindow* window = nullptr;
static GLuint program = 0;
static GLuint vao = 0;
static GLuint vbo = 0;

bool Renderer::init() {

    if (!glfwInit()) { 
        std::cerr << "GLFW init fallito\n"; 
        return false; 
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "3dAudioVisualizer", nullptr, nullptr);

    if (!window) { 
        std::cerr << "Creazione finestra fallita\n"; 
        return false;
    }
    
    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        std::cerr << "GLAD inizializzazione fallita\n"; 
        return false; 
    }
    std::cout << "GLAD OK | OpenGL: " << glGetString(GL_VERSION) << "\n";

    //SHADER INLINE, TOCCA MODIFICARE
    const char* vs = "#version 460 core\nlayout(location=0) in vec3 p;\nvoid main() { gl_Position = vec4(p, 1.0); }\n"; //Vert
    const char* fs = "#version 460 core\nout vec4 c; uniform float u_level;\nvoid main() { c = vec4(u_level, 0.3, 1.0 - u_level, 1.0); }\n"; //Frag

    GLuint vShader = loadShader(vs, GL_VERTEX_SHADER);
    GLuint fShader = loadShader(fs, GL_FRAGMENT_SHADER);

    program = glCreateProgram();
    glAttachShader(program, vShader); glAttachShader(program, fShader); glLinkProgram(program);
    glUseProgram(program);
    glDeleteShader(vShader); glDeleteShader(fShader);

    float verts[] = { -0.5f, -0.5f, 0.0f,  0.5f, -0.5f, 0.0f,  0.0f, 0.5f, 0.0f };

    createBuffers(verts);

    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    std::cout << "Rendering loop avviato...\n";
    return true;
}

void Renderer::createBuffers(float verts[]) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

GLuint Renderer::loadShader(const char* s, int type) {
    GLuint Shader = glCreateShader(type);
    glShaderSource(Shader, 1, &s, nullptr); 
    glCompileShader(Shader);
    return Shader;
}

void Renderer::shutdown() {
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Renderer::render() {
    float level = getAudioLevel();
    glUniform1f(glGetUniformLocation(program, "u_level"), level);

    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
}

float Renderer::getAudioLevel() {
    return AudioEngine::getAudioLevel().load(std::memory_order_relaxed);
}

GLFWwindow* Renderer::getWindow() {
    return window;
}
