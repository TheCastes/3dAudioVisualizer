#include "../../include/ui/Ui.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

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

void Ui::draw(const std::string& trackName, bool isPlaying) {
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(280.0f, 0.0f), ImGuiCond_Always);
    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("3dAudioVisualizer", nullptr, flags);

    if (ImGui::Button("Apri file...") && browseCallback)
        browseCallback();

    const bool hasTrack = !trackName.empty();

    ImGui::TextWrapped("%s", hasTrack ? trackName.c_str() : "Nessuna traccia");

    if (!hasTrack) ImGui::BeginDisabled();
    if (ImGui::Button(isPlaying ? "Pausa" : "Play") && playPauseCallback)
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