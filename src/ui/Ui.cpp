#include "../../include/ui/Ui.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <algorithm>
#include <filesystem>

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

void Ui::setLoadCallback(std::function<void(const std::string&)> callback) {
    loadCallback = std::move(callback);
}

void Ui::scanTracks(const std::string& directory) {
    tracks.clear();
    selectedTrack = -1;

    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::is_directory(directory, ec)) return;

    for (const auto& entry : fs::directory_iterator(directory, ec)) {
        if (!entry.is_regular_file()) continue;
        tracks.push_back({ entry.path().string(), entry.path().filename().string() });
    }

    std::sort(tracks.begin(), tracks.end(),
              [](const Track& a, const Track& b) { return a.name < b.name; });
}

void Ui::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Ui::draw() {
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(280.0f, 0.0f), ImGuiCond_Always);
    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("3dAudioVisualizer", nullptr, flags);

    const char* preview = (selectedTrack >= 0 && selectedTrack < static_cast<int>(tracks.size()))
        ? tracks[selectedTrack].name.c_str()
        : "Seleziona una traccia";

    if (ImGui::BeginCombo("Traccia", preview)) {
        for (int i = 0; i < static_cast<int>(tracks.size()); ++i) {
            const bool isSelected = (i == selectedTrack);
            if (ImGui::Selectable(tracks[i].name.c_str(), isSelected))
                selectedTrack = i;
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    const bool canLoad = selectedTrack >= 0;
    if (!canLoad) ImGui::BeginDisabled();
    if (ImGui::Button("Load") && loadCallback)
        loadCallback(tracks[selectedTrack].path);
    if (!canLoad) ImGui::EndDisabled();

    ImGui::End();
}

void Ui::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}