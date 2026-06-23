#include <iostream>

#include "../../include/renderer/Renderer.h"

Renderer::Renderer() = default;

Renderer::~Renderer() {
    if (!isInitialized) return;
    mesh.reset();
    shaderLibrary.clear();
    spectrogramTexture.reset();
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

    if (viewportHeight > 0)
        projectionMatrix = glm::perspective(glm::radians(45.0f),
            (float)viewportWidth / (float)viewportHeight, 0.1f, 10000.0f);

    glfwSetWindowUserPointer(applicationWindow, this);
    glfwSetMouseButtonCallback(applicationWindow, glfwMouseButtonCallback);
    glfwSetCursorPosCallback(applicationWindow, glfwCursorPosCallback);
    glfwSetKeyCallback(applicationWindow, glfwKeyCallback);

    glfwSetFramebufferSizeCallback(applicationWindow, [](GLFWwindow* window, const int width, const int height){
            glViewport(0, 0, width, height);
            Renderer* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
            self->viewportWidth = width;
            self->viewportHeight = height;
            if (height > 0)
                self->projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 10000.0f);
            });

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shaderLibrary.add("Displacement Heatmap", "../assets/shaders/displacement.vert", "../assets/shaders/heatmap.frag");
    shaderLibrary.add("Displacement B&W", "../assets/shaders/displacement.vert", "../assets/shaders/greyscale.frag");

    mesh = std::make_unique<Mesh>(512, 512, 10, 10);

    spectrogramTexture = std::make_unique<SpectrogramTexture>();
    spectrogramTexture->init();

    glClearColor(0.20f, 0.20f, 0.20f, 1.0f);
    std::cout << "Rendering loop avviato...\n";
    isInitialized = true;
    return true;
}

void Renderer::render(const float currentAudioLevel, const SpectrogramBuffer& spectrogramBuffer) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    spectrogramTexture->update(spectrogramBuffer);
    spectrogramTexture->bind(0);

    const int subW = static_cast<int>(viewportWidth  * renderFractionW);
    const int subH = static_cast<int>(viewportHeight * renderFractionH);
    glViewport(viewportWidth - subW, viewportHeight - subH, subW, subH);

    Shader& shader = shaderLibrary.active();
    shader.Use();
    shader.set("spectrogram", 0);
    shader.set("level", currentAudioLevel);

    if (shaderLibrary.activeIndex() == 0 && !colormapList.empty()) {
        const Colormap& cm = colormapList[activeColormapIndex];
        shader.set("colorStopPositions", cm.positions.data(), 5);
        shader.set("colorStopColors", cm.colors.data(), 5);
    }

    shader.set("projectionMatrix", projectionMatrix);
    shader.set("viewMatrix", viewMatrix);
    mesh->modelMatrix = trackball.rotationMatrix();
    shader.set("modelMatrix", mesh->modelMatrix);
    mesh->Draw();
}

void Renderer::swapBuffers() const {
    glfwSwapBuffers(applicationWindow);
}

bool Renderer::shouldClose() const {
    return glfwWindowShouldClose(applicationWindow);
}

bool Renderer::cursorToSubViewport(const double x, const double y, float& localX, float& localY,
                                   int& subW, int& subH) const {
    int winW, winH;
    glfwGetWindowSize(applicationWindow, &winW, &winH);
    subW = static_cast<int>(winW * renderFractionW);
    subH = static_cast<int>(winH * renderFractionH);
    localX = static_cast<float>(x) - static_cast<float>(winW - subW); // right aligned
    localY = static_cast<float>(y);                                   // top aligned
    return localX >= 0.0f && localX <= static_cast<float>(subW)
        && localY >= 0.0f && localY <= static_cast<float>(subH);
}

void Renderer::glfwMouseButtonCallback(GLFWwindow* window, const int button, const int action, int mods) {
    Renderer* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    float localX, localY;
    int subW, subH;
    const bool insideViewport = self->cursorToSubViewport(x, y, localX, localY, subW, subH);
    
    if (action == GLFW_PRESS && insideViewport)
        self->trackball.mouseDown(localX, localY, subW, subH);
    else if (action == GLFW_RELEASE)
        self->trackball.mouseUp();
}

void Renderer::glfwCursorPosCallback(GLFWwindow* window, const double x, const double y) {
    Renderer* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    float localX, localY;
    int subW, subH;
    self->cursorToSubViewport(x, y, localX, localY, subW, subH);
    self->trackball.mouseMove(localX, localY, subW, subH);
}

void Renderer::glfwKeyCallback(GLFWwindow* window, const int key, int scancode, const int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// this might be useful to let users import their own colormaps but who cares 
// void Renderer::addColormap(Colormap cm) {
//     colormapList.push_back(std::move(cm));
// }
