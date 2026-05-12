#include "../imgui/imgui.h"
#include "gui.h"

void GUI::ApplyStyle()
{
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding     = 6.0f;
    s.ChildRounding      = 4.0f;
    s.FrameRounding      = 4.0f;
    s.ScrollbarRounding  = 4.0f;
    s.GrabRounding       = 4.0f;
    s.TabRounding        = 4.0f;
    s.WindowBorderSize   = 1.0f;
    s.FrameBorderSize    = 0.0f;
    s.ItemSpacing        = ImVec2(8.f, 6.f);
    s.WindowPadding      = ImVec2(10.f, 10.f);
    s.ScrollbarSize      = 12.0f;

    ImVec4* c = s.Colors;
    c[ImGuiCol_Text]                  = ImVec4(0.85f, 0.88f, 0.95f, 1.00f);
    c[ImGuiCol_TextDisabled]          = ImVec4(0.40f, 0.45f, 0.55f, 1.00f);
    c[ImGuiCol_WindowBg]              = ImVec4(0.09f, 0.10f, 0.13f, 1.00f);
    c[ImGuiCol_ChildBg]               = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);
    c[ImGuiCol_PopupBg]               = ImVec4(0.09f, 0.10f, 0.13f, 1.00f);
    c[ImGuiCol_Border]                = ImVec4(0.22f, 0.28f, 0.40f, 0.60f);
    c[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    c[ImGuiCol_FrameBg]               = ImVec4(0.13f, 0.15f, 0.20f, 1.00f);
    c[ImGuiCol_FrameBgHovered]        = ImVec4(0.18f, 0.22f, 0.30f, 1.00f);
    c[ImGuiCol_FrameBgActive]         = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    c[ImGuiCol_TitleBg]               = ImVec4(0.07f, 0.08f, 0.11f, 1.00f);
    c[ImGuiCol_TitleBgActive]         = ImVec4(0.10f, 0.13f, 0.20f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.07f, 0.08f, 0.11f, 1.00f);
    c[ImGuiCol_MenuBarBg]             = ImVec4(0.09f, 0.10f, 0.13f, 1.00f);
    c[ImGuiCol_ScrollbarBg]           = ImVec4(0.07f, 0.08f, 0.11f, 1.00f);
    c[ImGuiCol_ScrollbarGrab]         = ImVec4(0.22f, 0.35f, 0.60f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.28f, 0.42f, 0.72f, 1.00f);
    c[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.35f, 0.50f, 0.85f, 1.00f);
    c[ImGuiCol_CheckMark]             = ImVec4(0.35f, 0.60f, 1.00f, 1.00f);
    c[ImGuiCol_SliderGrab]            = ImVec4(0.30f, 0.55f, 0.95f, 1.00f);
    c[ImGuiCol_SliderGrabActive]      = ImVec4(0.40f, 0.65f, 1.00f, 1.00f);
    c[ImGuiCol_Button]                = ImVec4(0.18f, 0.28f, 0.48f, 1.00f);
    c[ImGuiCol_ButtonHovered]         = ImVec4(0.25f, 0.40f, 0.65f, 1.00f);
    c[ImGuiCol_ButtonActive]          = ImVec4(0.30f, 0.48f, 0.78f, 1.00f);
    c[ImGuiCol_Header]                = ImVec4(0.18f, 0.28f, 0.48f, 1.00f);
    c[ImGuiCol_HeaderHovered]         = ImVec4(0.25f, 0.40f, 0.65f, 1.00f);
    c[ImGuiCol_HeaderActive]          = ImVec4(0.30f, 0.48f, 0.78f, 1.00f);
    c[ImGuiCol_Separator]             = ImVec4(0.22f, 0.28f, 0.40f, 0.80f);
    c[ImGuiCol_SeparatorHovered]      = ImVec4(0.30f, 0.45f, 0.70f, 1.00f);
    c[ImGuiCol_SeparatorActive]       = ImVec4(0.35f, 0.55f, 0.85f, 1.00f);
    c[ImGuiCol_ResizeGrip]            = ImVec4(0.20f, 0.35f, 0.60f, 0.60f);
    c[ImGuiCol_ResizeGripHovered]     = ImVec4(0.28f, 0.45f, 0.72f, 0.80f);
    c[ImGuiCol_ResizeGripActive]      = ImVec4(0.35f, 0.55f, 0.85f, 1.00f);
    c[ImGuiCol_Tab]                   = ImVec4(0.13f, 0.18f, 0.28f, 1.00f);
    c[ImGuiCol_TabHovered]            = ImVec4(0.25f, 0.40f, 0.65f, 1.00f);
    c[ImGuiCol_TabActive]             = ImVec4(0.20f, 0.32f, 0.55f, 1.00f);
    c[ImGuiCol_TabUnfocused]          = ImVec4(0.10f, 0.13f, 0.20f, 1.00f);
    c[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.15f, 0.22f, 0.38f, 1.00f);
    c[ImGuiCol_PlotLines]             = ImVec4(0.35f, 0.60f, 1.00f, 1.00f);
    c[ImGuiCol_PlotLinesHovered]      = ImVec4(0.50f, 0.75f, 1.00f, 1.00f);
    c[ImGuiCol_PlotHistogram]         = ImVec4(0.30f, 0.55f, 0.95f, 1.00f);
    c[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.40f, 0.65f, 1.00f, 1.00f);
    c[ImGuiCol_TableHeaderBg]         = ImVec4(0.13f, 0.18f, 0.28f, 1.00f);
    c[ImGuiCol_TableBorderStrong]     = ImVec4(0.22f, 0.28f, 0.40f, 1.00f);
    c[ImGuiCol_TableBorderLight]      = ImVec4(0.15f, 0.20f, 0.30f, 1.00f);
    c[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    c[ImGuiCol_TableRowBgAlt]         = ImVec4(0.13f, 0.15f, 0.20f, 0.50f);
    c[ImGuiCol_DragDropTarget]        = ImVec4(0.35f, 0.60f, 1.00f, 0.90f);
    c[ImGuiCol_NavHighlight]          = ImVec4(0.35f, 0.60f, 1.00f, 1.00f);
    c[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    c[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    c[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.10f, 0.10f, 0.15f, 0.60f);
}

void GUI::Render()
{
    ImGui::SetNextWindowSize(ImVec2(420.f, 320.f), ImGuiCond_Always);
    ImGui::Begin("cs2cheat", &g_Open,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings);

    if (ImGui::BeginTabBar("##tabs"))
    {
        if (ImGui::BeginTabItem("Legit"))
        {
            ImGui::Spacing();
            ImGui::Text("Aimbot");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::SliderFloat("FOV",    &fAimFOV,    0.5f, 30.0f, "%.1f deg");
            ImGui::SliderFloat("Smooth", &fAimSmooth, 1.0f, 20.0f, "%.1f");
            ImGui::Spacing();
            ImGui::Checkbox("Triggerbot", &bTriggerbot);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Visuals"))
        {
            ImGui::Spacing();
            ImGui::Checkbox("Glow ESP", &bGlowESP);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Misc"))
        {
            ImGui::Spacing();
            ImGui::Checkbox("Bunnyhop", &bBhop);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextDisabled("INSERT  - toggle menu");
            ImGui::TextDisabled("END     - unload cheat");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
