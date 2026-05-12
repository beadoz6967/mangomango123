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
    ImGui::SetNextWindowSize(ImVec2(500.f, 600.f), ImGuiCond_FirstUseEver);
    ImGui::Begin("LitWare CS2", &g_Open,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings);

    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
    {
        // ========== AIMBOT TAB ==========
        if (ImGui::BeginTabItem("Aimbot"))
        {
            ImGui::Spacing();
            ImGui::Checkbox("Enable Aimbot", &bAimbot);
            ImGui::Separator();

            if (bAimbot)
            {
                ImGui::SliderFloat("FOV##aim", &fAimFOV, 0.5f, 50.0f, "%.1f deg");
                ImGui::SliderFloat("Smoothing##aim", &fAimSmooth, 1.0f, 30.0f, "%.1f");

                const char* bones[] = { "Body", "Head" };
                ImGui::Combo("Target Bone", &iAimbotBone, bones, IM_ARRAYSIZE(bones));

                ImGui::Checkbox("Team Check##aim", &bAimbotTeamCheck);
                ImGui::Checkbox("Triggerbot##aim", &bTriggerbot);
            }

            ImGui::EndTabItem();
        }

        // ========== ESP TAB ==========
        if (ImGui::BeginTabItem("ESP"))
        {
            ImGui::Spacing();
            ImGui::Text("Rendering");
            ImGui::Separator();
            ImGui::Checkbox("Box ESP", &bBoxESP);
            ImGui::Checkbox("Health Bar", &bHealthBar);
            ImGui::Checkbox("Player Names", &bPlayerNames);
            ImGui::Checkbox("Weapon Info", &bWeaponInfo);
            ImGui::Checkbox("Distance", &bDistance);
            ImGui::Checkbox("Offscreen Arrows", &bArrows);

            ImGui::Spacing();
            ImGui::Text("Effects");
            ImGui::Separator();
            ImGui::Checkbox("Glow ESP", &bGlowESP);
            ImGui::Checkbox("Wallhack", &bWallhackESP);

            ImGui::EndTabItem();
        }

        // ========== VISUALS TAB ==========
        if (ImGui::BeginTabItem("Visuals"))
        {
            ImGui::Spacing();
            ImGui::Text("Visual Enhancements");
            ImGui::Separator();
            ImGui::Checkbox("No Flash", &bNoFlash);
            ImGui::Checkbox("No Smoke", &bNoSmoke);
            ImGui::Checkbox("Custom Sky", &bSkyColor);

            ImGui::Spacing();
            ImGui::Text("Camera");
            ImGui::Separator();
            ImGui::SliderFloat("FOV", &fFOVChanger, 60.0f, 120.0f, "%.0f");

            ImGui::EndTabItem();
        }

        // ========== MOVEMENT TAB ==========
        if (ImGui::BeginTabItem("Movement"))
        {
            ImGui::Spacing();
            ImGui::Text("Movement Assistance");
            ImGui::Separator();
            ImGui::Checkbox("Bunny Hop", &bBhop);
            ImGui::Checkbox("Strafe Helper", &bStrafe);

            ImGui::EndTabItem();
        }

        // ========== MISC TAB ==========
        if (ImGui::BeginTabItem("Misc"))
        {
            ImGui::Spacing();
            ImGui::Text("Overlay");
            ImGui::Separator();
            ImGui::Checkbox("Radar", &bRadar);
            ImGui::Checkbox("Bomb Timer", &bBombTimer);
            ImGui::Checkbox("Spectator List", &bSpectatorList);

            ImGui::Spacing();
            ImGui::Text("Settings");
            ImGui::Separator();
            if (ImGui::Button("Save Config", ImVec2(-1, 0)))
            {
                // Config will be saved on unload
            }
            if (ImGui::Button("Reset to Defaults", ImVec2(-1, 0)))
            {
                // Reset will be called from config
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextDisabled("INSERT  - toggle menu");
            ImGui::TextDisabled("END     - unload cheat");
            ImGui::TextDisabled("v1.0 - Powered by LitWare");

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
