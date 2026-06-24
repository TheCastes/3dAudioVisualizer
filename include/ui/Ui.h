#pragma once

#include <functional>
#include <string>
#include <vector>

#include "../renderer/Colormap.h"
#include "../renderer/RenderMode.h"
#include "../renderer/ShaderControls.h"
#include "../renderer/ShaderParameters.h"
#include "../renderer/SpectrogramScale.h"

struct GLFWwindow;


class Ui {
public:
    struct PlaybackState {
        std::string trackName;
        bool isPlaying = false;
        double positionSeconds = 0.0;
        double lengthSeconds = 0.0;
    };

    Ui();
    ~Ui();

    Ui(const Ui& copy) = delete;
    Ui& operator=(const Ui&) = delete;
    Ui(Ui&& move) = delete;
    Ui& operator=(Ui&&) = delete;

    bool init(GLFWwindow* window);

    void setBrowseCallback(std::function<void()> callback);
    void setPlayPauseCallback(std::function<void()> callback);
    void setStopCallback(std::function<void()> callback);
    void setRenderModeCallback(std::function<void(RenderMode)> callback);
    void setShaderCallback(std::function<void(int)> callback);
    void setColormapCallback(std::function<void(int)> callback);
    void setSpectrogramScaleCallback(std::function<void(SpectrogramScale)> callback);
    void setSpectrogramGainCallback(std::function<void(float)> callback);
    void setResetRotationCallback(std::function<void()> callback);
    void setResetParametersCallback(std::function<void()> callback);
    void setPresetIsometricCallback(std::function<void()> callback);
    void setPresetSpectrumCallback(std::function<void()> callback);

    void beginFrame();
    void draw(const PlaybackState& state, RenderMode renderMode, SpectrogramScale spectrogramScale,
              const ShaderControls& shaderControls);
    void render();

private:
    float drawPlayerPanel(const PlaybackState& state);
    float drawShaderPanel(float topY, RenderMode renderMode, const ShaderControls& shaderControls);
    void drawSpectrogramPanel(float topY, SpectrogramScale spectrogramScale);

    void fileButton(bool hasTrack);
    void trackInfo(const PlaybackState& state);
    void transportControls(bool hasTrack, bool isPlaying);

    void renderModeSelector(RenderMode renderMode);
    void spectrogramScaleSelector(SpectrogramScale spectrogramScale);
    void spectrogramGainSlider();
    void shaderSelector(const std::vector<std::string>& shaderNames, const std::vector<RenderMode>& shaderModes, int shaderIndex);
    void colormapDropdown(const std::vector<Colormap>& colormaps, int colormapIndex);
    void shaderParametersSection(RenderMode renderMode, const ShaderControls& shaderControls);
    bool initialized = false;

    std::function<void()> browseCallback;
    std::function<void()> playPauseCallback;
    std::function<void()> stopCallback;
    std::function<void(RenderMode)> renderModeCallback;
    std::function<void(int)> shaderCallback;
    std::function<void(int)> colormapCallback;
    std::function<void(SpectrogramScale)> spectrogramScaleCallback;
    std::function<void(float)> spectrogramGainCallback;
    std::function<void()> resetRotationCallback;
    std::function<void()> resetParametersCallback;
    std::function<void()> presetIsometricCallback;
    std::function<void()> presetSpectrumCallback;

    float spectrogramGainDecibels = 0.0f;
};