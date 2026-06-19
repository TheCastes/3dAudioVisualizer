#include "../../include/ui/Ui.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <cstdio>

namespace {
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

void Ui::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Ui::draw(const PlaybackState& state) {
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(280.0f, 0.0f), ImGuiCond_Always);
    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("3dAudioVisualizer", nullptr, flags);

    const bool hasTrack = !state.trackName.empty();

    if (hasTrack) ImGui::BeginDisabled();
    if (ImGui::Button("Apri file...") && browseCallback)
        browseCallback();
    if (hasTrack) ImGui::EndDisabled();

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

    if (!hasTrack) ImGui::BeginDisabled();
    if (ImGui::Button(state.isPlaying ? "Pausa" : "Play") && playPauseCallback)
        playPauseCallback();
    ImGui::SameLine();
    if (ImGui::Button("Stop") && stopCallback)
        stopCallback();
    if (!hasTrack) ImGui::EndDisabled();

    ImGui::End();
}

void Ui::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}