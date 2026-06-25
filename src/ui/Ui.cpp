#include "ui/Ui.h"
#include "ui/Theme.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <cstdio>

namespace {
    struct PanelLayout {
        float x = 0.0f;
        float width = 450.0f;
        float gap = 8.0f;
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    };
    constexpr PanelLayout panel{};

    std::string formatTime(double seconds) {
        if (seconds < 0.0) seconds = 0.0;
        const int total = static_cast<int>(seconds);
        char buffer[16];
        std::snprintf(buffer, sizeof(buffer), "%d:%02d", total / 60, total % 60);
        return buffer;
    }
}

Ui::Ui() = default;

Ui::~Ui() {
    if (!initialized) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool Ui::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    applyTheme();
    
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) return false;
    if (!ImGui_ImplOpenGL3_Init("#version 460")) return false;

    initialized = true;
    return true;
}

void Ui::setBrowseCallback(std::function<void()> callback) {
    browseCallback = std::move(callback);
}

void Ui::setPlayPauseCallback(std::function<void()> callback) {
    playPauseCallback = std::move(callback);
}

void Ui::setStopCallback(std::function<void()> callback) {
    stopCallback = std::move(callback);
}

void Ui::setRenderModeCallback(std::function<void(RenderMode)> callback) {
    renderModeCallback = std::move(callback);
}

void Ui::setColormapCallback(std::function<void(int)> callback) {
    colormapCallback = std::move(callback);
}

void Ui::setShaderCallback(std::function<void(int)> callback) {
    shaderCallback = std::move(callback);
}

void Ui::setSpectrogramScaleCallback(std::function<void(SpectrogramScale)> callback) {
    spectrogramScaleCallback = std::move(callback);
}

void Ui::setSpectrogramGainCallback(std::function<void(float)> callback) {
    spectrogramGainCallback = std::move(callback);
}

void Ui::setResetRotationCallback(std::function<void()> callback) {
    resetRotationCallback = std::move(callback);
}

void Ui::setResetParametersCallback(std::function<void()> callback) {
    resetParametersCallback = std::move(callback);
}

void Ui::setPresetIsometricCallback(std::function<void()> callback) {
    presetIsometricCallback = std::move(callback);
}

void Ui::setPresetSpectrumCallback(std::function<void()> callback) {
    presetSpectrumCallback = std::move(callback);
}

void Ui::fileButton(bool hasTrack) {
    if (hasTrack) ImGui::BeginDisabled();
    if (ImGui::Button("Open file...") && browseCallback)
        browseCallback();
    if (hasTrack) ImGui::EndDisabled();
}

void Ui::trackInfo(const PlaybackState& state) {
    const bool hasTrack = !state.trackName.empty();
    ImGui::TextUnformatted(hasTrack ? state.trackName.c_str() : "No track");
    ImGui::SameLine();
    const std::string timeText = formatTime(state.positionSeconds) + " / " + formatTime(state.lengthSeconds);
    ImGui::TextUnformatted(timeText.c_str());

    const float fraction = state.lengthSeconds > 0.0
        ? static_cast<float>(state.positionSeconds / state.lengthSeconds)
        : 0.0f;
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.45f, 0.70f, 0.50f, 1.0f));
    ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), "");
    ImGui::PopStyleColor();
}

void Ui::transportControls(bool hasTrack, bool isPlaying) {
    if (!hasTrack) ImGui::BeginDisabled();
    if (ImGui::Button(isPlaying ? "Pause" : "Play") && playPauseCallback)
        playPauseCallback();
    ImGui::SameLine();
    if (ImGui::Button("Stop") && stopCallback)
        stopCallback();
    if (!hasTrack) ImGui::EndDisabled();
}

void Ui::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Ui::draw(const PlaybackState& state, RenderMode renderMode, SpectrogramScale spectrogramScale,
              const ShaderControls& shaderControls) {
    const float playerBottom = drawPlayerPanel(state);
    const float shaderBottom = drawShaderPanel(playerBottom + panel.gap, renderMode, shaderControls);
    drawSpectrogramPanel(shaderBottom + panel.gap, spectrogramScale);
}

float Ui::drawPlayerPanel(const PlaybackState& state) {
    ImGui::SetNextWindowPos(ImVec2(panel.x, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panel.width, 0.0f), ImGuiCond_Always);

    ImGui::Begin("Player", nullptr, panel.flags);

    const bool hasTrack = !state.trackName.empty();
    fileButton(hasTrack);
    trackInfo(state);
    transportControls(hasTrack, state.isPlaying);

    const float bottom = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
    ImGui::End();
    return bottom;
}

float Ui::drawShaderPanel(float topY, RenderMode renderMode, const ShaderControls& shaderControls) {
    ImGui::SetNextWindowPos(ImVec2(panel.x, topY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panel.width, 0.0f), ImGuiCond_Always);

    ImGui::Begin("Render modes", nullptr, panel.flags);

    renderModeSelector(renderMode);
    shaderParametersSection(renderMode, shaderControls);

    const float bottom = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
    ImGui::End();
    return bottom;
}

void Ui::drawSpectrogramPanel(float topY, SpectrogramScale spectrogramScale) {
    ImGui::SetNextWindowPos(ImVec2(panel.x, topY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panel.width, 0.0f), ImGuiCond_Always);

    ImGui::Begin("Spectrogram settings", nullptr, panel.flags);

    spectrogramScaleSelector(spectrogramScale);
    spectrogramGainSlider();

    ImGui::End();
}

void Ui::renderModeSelector(RenderMode renderMode) {
    if (ImGui::RadioButton("Scientific", renderMode == RenderMode::Scientific) && renderModeCallback)
        renderModeCallback(RenderMode::Scientific);
    ImGui::SameLine();
    if (ImGui::RadioButton("Spherical", renderMode == RenderMode::Spherical) && renderModeCallback)
        renderModeCallback(RenderMode::Spherical);
}

void Ui::spectrogramScaleSelector(SpectrogramScale spectrogramScale) {
    if (ImGui::RadioButton("Linear", spectrogramScale == SpectrogramScale::Linear) && spectrogramScaleCallback)
        spectrogramScaleCallback(SpectrogramScale::Linear);
    ImGui::SameLine();
    if (ImGui::RadioButton("Mel", spectrogramScale == SpectrogramScale::Mel) && spectrogramScaleCallback)
        spectrogramScaleCallback(SpectrogramScale::Mel);
}

void Ui::spectrogramGainSlider() {
    if (ImGui::SliderFloat("Gain (dB)", &spectrogramGainDecibels, -24.0f, 24.0f, "%.1f") && spectrogramGainCallback)
        spectrogramGainCallback(spectrogramGainDecibels);
}

void Ui::shaderSelector(const std::vector<std::string>& shaderNames, const std::vector<RenderMode>& shaderModes, int shaderIndex) {
    bool first = true;
    for (int i = 0; i < static_cast<int>(shaderNames.size()); ++i) {
        if (shaderModes[i] != RenderMode::Spherical) continue;
        if (!first) ImGui::SameLine();
        first = false;
        if (ImGui::RadioButton(shaderNames[i].c_str(), shaderIndex == i) && shaderCallback)
            shaderCallback(i);
    }
}

void Ui::colormapDropdown(const std::vector<Colormap>& colormaps, int colormapIndex) {
    ImGui::Text("Colormap");
    const char* currentName = colormaps[colormapIndex].name.c_str();
    if (ImGui::BeginCombo("##colormap", currentName)) {
        for (int i = 0; i < static_cast<int>(colormaps.size()); ++i) {
            bool isSelected = (colormapIndex == i);
            if (ImGui::Selectable(colormaps[i].name.c_str(), isSelected)) {
                if (colormapCallback)
                    colormapCallback(i);
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

void Ui::shaderParametersSection(RenderMode renderMode, const ShaderControls& shaderControls) {
    ShaderParameters& parameters = shaderControls.parameters;
    ImGui::SeparatorText("View presets");
    if (ImGui::Button("Top") && resetRotationCallback)
        resetRotationCallback();
    ImGui::SameLine();
    if (ImGui::Button("Isometric") && presetIsometricCallback)
        presetIsometricCallback();
    ImGui::SameLine();
    if (ImGui::Button("Spectrum") && presetSpectrumCallback)
        presetSpectrumCallback();
    ImGui::SeparatorText("Shader settings");
    if (!shaderControls.colormaps.empty())
        colormapDropdown(shaderControls.colormaps, shaderControls.colormapIndex);
    if (renderMode == RenderMode::Spherical) {
        shaderSelector(shaderControls.shaderNames, shaderControls.shaderModes, shaderControls.shaderIndex);
        ImGui::SliderInt("Sphere grid size", &parameters.sphereGridSize, 10, 200);
        ImGui::SliderFloat("Height scale", &parameters.heightScale, 0.0f, 5.0f, "%.2f");
    }
    ImGui::SliderInt("Temporal window", &parameters.temporalWindow, 1, 32);
    ImGui::SliderFloat("Temporal sigma", &parameters.temporalSigma, 1.0f, 60.0f, "%.1f");
    ImGui::SliderInt("Freq smoothing", &parameters.freqSampleSize, 1, 8);
    if (renderMode == RenderMode::Spherical) {
        ImGui::SliderFloat("Base radius", &parameters.baseRadius, 0.0f, 0.2f, "%.3f");
        ImGui::SliderFloat("Radius scale", &parameters.radiusScale, 0.0f, 0.5f, "%.3f");
    }
    ImGui::Separator();
    if (ImGui::Button("Reset parameters") && resetParametersCallback)
        resetParametersCallback();
}

void Ui::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}