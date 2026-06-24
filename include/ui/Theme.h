#pragma once

#include <imgui.h>

inline void applyTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    style.FrameRounding = 4.0f;
    style.WindowRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    style.FramePadding = ImVec2(8.0f, 6.0f);
    style.ItemSpacing = ImVec2(8.0f, 8.0f);
    style.WindowPadding = ImVec2(12.0f, 10.0f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.15f, 0.17f, 0.22f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.13f, 0.17f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.20f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.27f, 0.36f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.14f, 0.16f, 0.21f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.32f, 0.45f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.15f, 0.17f, 0.22f, 0.50f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.50f, 0.70f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.62f, 0.82f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.32f, 0.45f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.40f, 0.55f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.14f, 0.16f, 0.21f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.32f, 0.45f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.60f, 0.85f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.09f, 0.09f, 0.12f, 0.96f);
    colors[ImGuiCol_Text] = ImVec4(0.80f, 0.84f, 0.90f, 1.00f);
}
