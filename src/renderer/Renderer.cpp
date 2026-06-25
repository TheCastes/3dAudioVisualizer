#include <iostream>

#include "renderer/Renderer.h"
#include "renderer/GridMesh.h"
#include "renderer/SphereFieldMesh.h"

Renderer::Renderer() = default;

Renderer::~Renderer() {
    if (!isInitialized) return;
    gridMesh.reset();
    sphereMesh.reset();
    shaderLibrary.clear();
    linearSpectrogramTexture.reset();
    melSpectrogramTexture.reset();
    glfwDestroyWindow(applicationWindow);
    glfwTerminate();
}

bool Renderer::init() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    applicationWindow = glfwCreateWindow(screenWidth, screenHeight, "3dAudioVisualizer", nullptr, nullptr);
    if (!applicationWindow) {
        std::cerr << "Window creation failed\n";
        return false;
    }

    glfwMakeContextCurrent(applicationWindow);

    if (!gladLoadGL()) {
        std::cerr << "GLAD initialization failed\n";
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
    glfwSetScrollCallback(applicationWindow, glfwScrollCallback);

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

#ifdef ASSETS_DIR
    const std::string shaderDir = std::string(ASSETS_DIR) + "/shaders/";
#else
    const std::string shaderDir = "../assets/shaders/";
#endif

    shaderLibrary.add("Displacement Heatmap", (shaderDir + "displacement.vert").c_str(), (shaderDir + "heatmap.frag").c_str(), RenderMode::Scientific);
    shaderLibrary.add("Sphere Height", (shaderDir + "sphere_height.vert").c_str(), (shaderDir + "sphere.frag").c_str(), RenderMode::Spherical);
    shaderLibrary.add("Sphere Radius", (shaderDir + "sphere_radius.vert").c_str(), (shaderDir + "sphere.frag").c_str(), RenderMode::Spherical);

    gridMesh = std::make_unique<GridMesh>(512, 512, 7, 7);
    sphereMesh = std::make_unique<SphereFieldMesh>(shaderParameters.sphereGridSize, 7, 7);
    previousSphereGridSize = shaderParameters.sphereGridSize;

    linearSpectrogramTexture = std::make_unique<SpectrogramTexture>();
    melSpectrogramTexture = std::make_unique<SpectrogramTexture>();

    std::cout << "Rendering loop started...\n";
    isInitialized = true;
    return true;
}

void Renderer::render(const SpectrogramBuffer& linearSpectrogram, const SpectrogramBuffer& melSpectrogram) {
    glViewport(0, 0, viewportWidth, viewportHeight);

    glClearColor(0.08f, 0.08f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // sub-viewport
    const int subWidth = static_cast<int>(viewportWidth  * renderFractionW);
    const int subHeight = static_cast<int>(viewportHeight * renderFractionH);

    glViewport(viewportWidth - subWidth, viewportHeight - subHeight, subWidth, subHeight);
    glEnable(GL_SCISSOR_TEST);
    glScissor(viewportWidth - subWidth, viewportHeight - subHeight, subWidth, subHeight);

    glClearColor(0.04f, 0.04f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SpectrogramTexture& activeTexture =
        (spectrogramScale == SpectrogramScale::Mel) ? *melSpectrogramTexture : *linearSpectrogramTexture;
    const SpectrogramBuffer& activeSpectrogram =
        (spectrogramScale == SpectrogramScale::Mel) ? melSpectrogram : linearSpectrogram;

    activeTexture.update(activeSpectrogram);
    activeTexture.bind(0);

    Shader& shader = shaderLibrary.getActiveShader();
    shader.use();
    shader.set("spectrogram", 0);
    shader.set("writeCursor", activeTexture.getWriteCursor());
    shader.set("validFrames", activeTexture.getValidFrameCount());
    shader.set("temporalWindow", shaderParameters.temporalWindow);
    shader.set("temporalSigma", shaderParameters.temporalSigma);
    shader.set("freqSampleSize", shaderParameters.freqSampleSize);

    if (!colormapList.empty()) {
        const Colormap& colormap = colormapList[activeColormapIndex];
        shader.set("colorStopPositions", colormap.positions.data(), 5);
        shader.set("colorStopColors", colormap.colors.data(), 5);
    }

    shader.set("projectionMatrix", projectionMatrix);
    shader.set("viewMatrix", viewMatrix * glm::scale(glm::mat4(1.0f), glm::vec3(trackball.getZoom())));

    if (renderMode == RenderMode::Spherical) {
        if (shaderParameters.sphereGridSize != previousSphereGridSize) {
            sphereMesh = std::make_unique<SphereFieldMesh>(shaderParameters.sphereGridSize, 7, 7);
            previousSphereGridSize = shaderParameters.sphereGridSize;
        }
        shader.set("heightScale", shaderParameters.heightScale);
        shader.set("baseRadius", shaderParameters.baseRadius);
        shader.set("radiusScale", shaderParameters.radiusScale);
    }

    Mesh& activeMesh = (renderMode == RenderMode::Scientific) ? *gridMesh : *sphereMesh;
    activeMesh.modelMatrix = trackball.getRotationMatrix();
    shader.set("modelMatrix", activeMesh.modelMatrix);
    activeMesh.Draw();

    glDisable(GL_SCISSOR_TEST);
}

void Renderer::swapBuffers() const {
    glfwSwapBuffers(applicationWindow);
}

bool Renderer::shouldClose() const {
    return glfwWindowShouldClose(applicationWindow);
}

bool Renderer::cursorToSubViewport(const double x, const double y, float& localX, float& localY,
                                   int& subWidth, int& subHeight) const {
    int windowWidth, windowHeight;
    glfwGetWindowSize(applicationWindow, &windowWidth, &windowHeight);
    subWidth = static_cast<int>(windowWidth * renderFractionW);
    subHeight = static_cast<int>(windowHeight * renderFractionH);
    localX = static_cast<float>(x) - static_cast<float>(windowWidth - subWidth); // right aligned
    localY = static_cast<float>(y);                                   // top aligned
    return localX >= 0.0f && localX <= static_cast<float>(subWidth)
        && localY >= 0.0f && localY <= static_cast<float>(subHeight);
}

void Renderer::glfwMouseButtonCallback(GLFWwindow* window, const int button, const int action, int mods) {
    Renderer* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    float localX, localY;
    int subWidth, subHeight;
    const bool insideViewport = self->cursorToSubViewport(x, y, localX, localY, subWidth, subHeight);
    
    if (action == GLFW_PRESS && insideViewport)
        self->trackball.mouseDown(localX, localY, subWidth, subHeight);
    else if (action == GLFW_RELEASE)
        self->trackball.mouseUp();
}

void Renderer::glfwCursorPosCallback(GLFWwindow* window, const double x, const double y) {
    Renderer* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    float localX, localY;
    int subWidth, subHeight;
    self->cursorToSubViewport(x, y, localX, localY, subWidth, subHeight);
    self->trackball.mouseMove(localX, localY, subWidth, subHeight);
}

void Renderer::glfwKeyCallback(GLFWwindow* window, const int key, int scancode, const int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Renderer::glfwScrollCallback(GLFWwindow* window, double /*xoffset*/, const double yoffset) {
    Renderer* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    self->trackball.scroll(static_cast<float>(yoffset));
}

void Renderer::setRenderMode(RenderMode mode) {
    renderMode = mode;
    shaderLibrary.setActive(shaderLibrary.firstIndexForMode(mode));
}

RenderMode Renderer::getRenderMode() const {
    return renderMode;
}
