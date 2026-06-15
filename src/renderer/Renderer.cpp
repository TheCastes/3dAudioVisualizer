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

    applicationWindow = glfwCreateWindow(screenWidth, screenHeight, "3dAudioVisualizer", nullptr, nullptr);
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

    glfwGetFramebufferSize(applicationWindow, &viewportWidth, &viewportHeight);
    glViewport(0, 0, viewportWidth, viewportHeight);

    glfwSetWindowUserPointer(applicationWindow, this);
    glfwSetMouseButtonCallback(applicationWindow, glfwMouseButtonCallback);
    glfwSetCursorPosCallback(applicationWindow, glfwCursorPosCallback);

    glfwSetFramebufferSizeCallback(applicationWindow, [](GLFWwindow* window, const int width, const int height){
            glViewport(0, 0, width, height);
            Renderer* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
            self->viewportWidth = width;
            self->viewportHeight = height;
            });

    glEnable(GL_DEPTH_TEST);

    shader = new Shader("../assets/shaders/shader.vert", "../assets/shaders/shader.frag");
    shader->Use();

    mesh = new Mesh(128, 128, 10.0f, 10.0f);
    // transform();
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    std::cout << "Rendering loop avviato...\n";
    return true;
}

void Renderer::render(const float currentAudioLevel) const {
    glUniform1f(glGetUniformLocation(shader->Program, "u_level"), currentAudioLevel);
    glClear(GL_COLOR_BUFFER_BIT);
    glUniformMatrix4fv(glGetUniformLocation(shader->Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader->Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    mesh->Draw();
    // centerView()
    mesh->modelMatrix = trackball.rotationMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shader->Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(mesh->modelMatrix));
    glfwSwapBuffers(applicationWindow);
}

bool Renderer::shouldClose() const {
    return glfwWindowShouldClose(applicationWindow);
}

// void Renderer::transform() const {
//     mesh->normalMatrix = glm::inverseTranspose(glm::mat3(viewMatrix*mesh->modelMatrix));
//     glUniformMatrix4fv(glGetUniformLocation(shader->Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(mesh->modelMatrix));
//     glUniformMatrix3fv(glGetUniformLocation(shader->Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(mesh->normalMatrix));
// }

void Renderer::glfwMouseButtonCallback(GLFWwindow* window, const int button, const int action, int mods) {
    Renderer* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (action == GLFW_PRESS)
        self->trackball.mouseDown(static_cast<float>(x), static_cast<float>(y), self->viewportWidth, self->viewportHeight);
    else if (action == GLFW_RELEASE)
        self->trackball.mouseUp();
}

void Renderer::glfwCursorPosCallback(GLFWwindow* window, const double x, const double y) {
    Renderer* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    self->trackball.mouseMove(static_cast<float>(x), static_cast<float>(y), self->viewportWidth, self->viewportHeight);
}
