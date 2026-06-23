#include "../../include/ui/Ui.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <cstdio>

namespace {
    struct PanelLayout {
        float x = 0.0f;
        float width = 280.0f;
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
    ImGui::StyleColorsDark();
    
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

void Ui::fileButton(bool hasTrack) {
    if (hasTrack) ImGui::BeginDisabled();
    if (ImGui::Button("Apri file...") && browseCallback)
        browseCallback();
    if (hasTrack) ImGui::EndDisabled();
}

void Ui::trackInfo(const PlaybackState& state) {
    const bool hasTrack = !state.trackName.empty();
    ImGui::TextUnformatted(hasTrack ? state.trackName.c_str() : "Nessuna traccia");
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
    if (ImGui::Button(isPlaying ? "Pausa" : "Play") && playPauseCallback)
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

void Ui::draw(const PlaybackState& state, RenderMode renderMode,
              const std::vector<std::string>& shaderNames, const std::vector<RenderMode>& shaderModes,
              int shaderIndex, const std::vector<Colormap>& colormaps, int colormapIndex) {
    const float playerBottom = drawPlayerPanel(state);
    drawShaderPanel(playerBottom + panel.gap, renderMode, shaderNames, shaderModes, shaderIndex, colormaps, colormapIndex);
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

void Ui::drawShaderPanel(float topY, RenderMode renderMode,
                         const std::vector<std::string>& shaderNames, const std::vector<RenderMode>& shaderModes,
                         int shaderIndex, const std::vector<Colormap>& colormaps, int colormapIndex) {
    ImGui::SetNextWindowPos(ImVec2(panel.x, topY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panel.width, 0.0f), ImGuiCond_Always);

    ImGui::Begin("Render modes", nullptr, panel.flags);

    renderModeSelector(renderMode);
    if (!colormaps.empty())
        colormapDropdown(colormaps, colormapIndex);
    if (renderMode == RenderMode::Spherical)
        shaderSelector(shaderNames, shaderModes, shaderIndex);

    ImGui::End();
}

void Ui::renderModeSelector(RenderMode renderMode) {
    if (ImGui::RadioButton("Scientific", renderMode == RenderMode::Scientific) && renderModeCallback)
        renderModeCallback(RenderMode::Scientific);
    ImGui::SameLine();
    if (ImGui::RadioButton("Spherical", renderMode == RenderMode::Spherical) && renderModeCallback)
        renderModeCallback(RenderMode::Spherical);
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

void Ui::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}