#include <iostream>

#include "../../include/renderer/Renderer.h"

Renderer::Renderer() = default;

Renderer::~Renderer() {
    if (!isInitialized) return;
    if (shader) {
        shader->Delete();
        delete shader;
        shader = nullptr;
    }
    glDeleteBuffers(1, &vertexBufferObject);
    glDeleteVertexArrays(1, &vertexArrayObject);
    glfwDestroyWindow(applicationWindow);
    glfwTerminate();
}

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
        glfwDestroyWindow(applicationWindow);
        applicationWindow = nullptr;
        glfwTerminate();
        return false; 
    }

    std::cout << "GLAD OK | OpenGL: " << glGetString(GL_VERSION) << "\n";

    shader = new Shader("../assets/shaders/shader.vert", "../assets/shaders/shader.frag");
    shader->Use();

    mesh = new Mesh(128, 128, 10.0f, 10.0f);

    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    std::cout << "Rendering loop avviato...\n";
    return true;
}

void Renderer::render(const float currentAudioLevel) const {

    glUniform1f(glGetUniformLocation(shader->Program, "u_level"), currentAudioLevel);
    glClear(GL_COLOR_BUFFER_BIT);
    mesh->Draw();
    glfwSwapBuffers(applicationWindow);
}

bool Renderer::shouldClose() const {
    return glfwWindowShouldClose(applicationWindow);
}