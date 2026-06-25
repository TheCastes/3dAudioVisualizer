#pragma once

#include <memory>
#include <glm/gtc/matrix_transform.hpp>

#include "Colormap.h"
#include "RenderMode.h"
#include "ShaderControls.h"
#include "ShaderLibrary.h"
#include "ShaderParameters.h"
#include "SpectrogramScale.h"
#include "Trackball.h"

struct GLFWwindow;
class Mesh;
class SpectrogramTexture;
class SpectrogramBuffer;

class Renderer {
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer& copy) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&& move) = delete;
    Renderer& operator=(Renderer&&) noexcept = delete;

    bool init();
    void render(const SpectrogramBuffer& linearSpectrogram, const SpectrogramBuffer& melSpectrogram);

    void pollEvents() const;
    void swapBuffers() const;
    bool shouldClose() const;

    void setRenderMode(RenderMode mode);
    RenderMode getRenderMode() const;

    void setSpectrogramScale(SpectrogramScale scale) { spectrogramScale = scale; }
    SpectrogramScale getSpectrogramScale() const { return spectrogramScale; }

    static void glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    static void glfwCursorPosCallback(GLFWwindow *window, double x, double y);

    static void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    static void glfwScrollCallback(GLFWwindow *window, double xoffset, double yoffset);

    GLFWwindow* getWindow() const { return applicationWindow; }

    void setActiveShader(int index) { shaderLibrary.setActive(index); }

    void setActiveColormap(int index) {
        if (index >= 0 && index < static_cast<int>(colormapList.size()))
            activeColormapIndex = index;
    }

    void resetRotation() { trackball.reset(); }
    void resetParameters() { shaderParameters = ShaderParameters{}; }
    void setPresetIsometric() {
        trackball.reset();
        trackball.applyRotation(-45.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        trackball.applyRotation(-45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    void setPresetSpectrum() {
        trackball.reset();
        trackball.applyRotation(-90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        trackball.applyRotation(-90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    }

    ShaderControls getShaderControls() {
        return { shaderLibrary.getNames(), shaderLibrary.getModes(), shaderLibrary.getActiveIndex(),
                 colormapList, activeColormapIndex, shaderParameters };
    }

private:
    glm::mat4 projectionMatrix = glm::mat4(1.0f);

    const int screenWidth = 1280;
    const int screenHeight = 720;

    static constexpr float renderFractionW = 0.7f;
    static constexpr float renderFractionH = 0.7f;

    int viewportWidth = 0;
    int viewportHeight = 0;

    bool cursorToSubViewport(double x, double y, float& localX, float& localY, int& subWidth, int& subHeight) const;

    void clearWindowBackground();
    void beginSubViewport();
    void endSubViewport();
    void refreshSphereMeshIfNeeded();

    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    GLFWwindow* applicationWindow = nullptr;
    ShaderLibrary shaderLibrary;
    std::unique_ptr<Mesh> gridMesh;
    std::unique_ptr<Mesh> sphereMesh;
    std::unique_ptr<SpectrogramTexture> linearSpectrogramTexture;
    std::unique_ptr<SpectrogramTexture> melSpectrogramTexture;
    SpectrogramScale spectrogramScale = SpectrogramScale::Linear;
    Trackball trackball;
    bool isInitialized = false;

    std::vector<Colormap> colormapList = defaultColormaps();
    int activeColormapIndex = 0;
    RenderMode renderMode = RenderMode::Scientific;
    ShaderParameters shaderParameters;
    int previousSphereGridSize = -1;
};
