// gui/gui.cpp — fixed: Save Config and Reset buttons actually do things now
#include "../imgui/imgui.h"
#include "gui.h"
#include "../core/config.hpp"

void GUI::ApplyStyle() {
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = 6.f; s.ChildRounding = 4.f; s.FrameRounding = 4.f;
    s.ScrollbarRounding = 4.f; s.GrabRounding = 4.f; s.TabRounding = 4.f;
    s.WindowBorderSize = 1.f; s.FrameBorderSize = 0.f;
    s.ItemSpacing = { 8.f, 6.f }; s.WindowPadding = { 10.f, 10.f }; s.ScrollbarSize = 12.f;

    ImVec4* c = s.Colors;
    c[ImGuiCol_Text]                  = {0.85f,0.88f,0.95f,1.f};
    c[ImGuiCol_TextDisabled]          = {0.40f,0.45f,0.55f,1.f};
    c[ImGuiCol_WindowBg]              = {0.09f,0.10f,0.13f,1.f};
    c[ImGuiCol_ChildBg]               = {0.10f,0.12f,0.15f,1.f};
    c[ImGuiCol_PopupBg]               = {0.09f,0.10f,0.13f,1.f};
    c[ImGuiCol_Border]                = {0.22f,0.28f,0.40f,0.60f};
    c[ImGuiCol_FrameBg]               = {0.13f,0.15f,0.20f,1.f};
    c[ImGuiCol_FrameBgHovered]        = {0.18f,0.22f,0.30f,1.f};
    c[ImGuiCol_FrameBgActive]         = {0.20f,0.25f,0.35f,1.f};
    c[ImGuiCol_TitleBg]               = {0.07f,0.08f,0.11f,1.f};
    c[ImGuiCol_TitleBgActive]         = {0.10f,0.13f,0.20f,1.f};
    c[ImGuiCol_CheckMark]             = {0.35f,0.60f,1.00f,1.f};
    c[ImGuiCol_SliderGrab]            = {0.30f,0.55f,0.95f,1.f};
    c[ImGuiCol_SliderGrabActive]      = {0.40f,0.65f,1.00f,1.f};
    c[ImGuiCol_Button]                = {0.18f,0.28f,0.48f,1.f};
    c[ImGuiCol_ButtonHovered]         = {0.25f,0.40f,0.65f,1.f};
    c[ImGuiCol_ButtonActive]          = {0.30f,0.48f,0.78f,1.f};
    c[ImGuiCol_Header]                = {0.18f,0.28f,0.48f,1.f};
    c[ImGuiCol_HeaderHovered]         = {0.25f,0.40f,0.65f,1.f};
    c[ImGuiCol_HeaderActive]          = {0.30f,0.48f,0.78f,1.f};
    c[ImGuiCol_Separator]             = {0.22f,0.28f,0.40f,0.80f};
    c[ImGuiCol_Tab]                   = {0.13f,0.18f,0.28f,1.f};
    c[ImGuiCol_TabHovered]            = {0.25f,0.40f,0.65f,1.f};
    c[ImGuiCol_TabActive]             = {0.20f,0.32f,0.55f,1.f};
    c[ImGuiCol_TabUnfocused]          = {0.10f,0.13f,0.20f,1.f};
    c[ImGuiCol_TabUnfocusedActive]    = {0.15f,0.22f,0.38f,1.f};
}

void GUI::Render() {
    ImGui::SetNextWindowSize(ImVec2(500.f, 600.f), ImGuiCond_FirstUseEver);
    ImGui::Begin("LitWare CS2", &g_Open,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

    if (ImGui::BeginTabBar("##tabs")) {

        if (ImGui::BeginTabItem("Aimbot")) {
            ImGui::Spacing();
            ImGui::Checkbox("Enable Aimbot", &bAimbot);
            ImGui::Separator();
            if (bAimbot) {
                ImGui::SliderFloat("FOV##aim",       &fAimFOV,    0.5f, 50.f, "%.1f deg");
                ImGui::SliderFloat("Smoothing##aim", &fAimSmooth, 1.f,  30.f, "%.1f");
                const char* bones[] = { "Body", "Head" };
                ImGui::Combo("Target Bone", &iAimbotBone, bones, IM_ARRAYSIZE(bones));
                ImGui::Checkbox("Team Check##aim", &bAimbotTeamCheck);
                ImGui::Checkbox("Triggerbot##aim",  &bTriggerbot);
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("ESP")) {
            ImGui::Spacing(); ImGui::Text("Rendering"); ImGui::Separator();
            ImGui::Checkbox("Box ESP",         &bBoxESP);
            ImGui::Checkbox("Health Bar",      &bHealthBar);
            ImGui::Checkbox("Player Names",    &bPlayerNames);
            ImGui::Checkbox("Weapon Info",     &bWeaponInfo);
            ImGui::Checkbox("Distance",        &bDistance);
            ImGui::Checkbox("Offscreen Arrows",&bArrows);
            ImGui::Spacing(); ImGui::Text("Effects"); ImGui::Separator();
            ImGui::Checkbox("Glow ESP",  &bGlowESP);
            ImGui::Checkbox("Wallhack",  &bWallhackESP);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Visuals")) {
            ImGui::Spacing(); ImGui::Text("Visual Enhancements"); ImGui::Separator();
            ImGui::Checkbox("No Flash",  &bNoFlash);
            ImGui::Checkbox("No Smoke",  &bNoSmoke);
            ImGui::Checkbox("Custom Sky",&bSkyColor);
            ImGui::Spacing(); ImGui::Text("Camera"); ImGui::Separator();
            ImGui::SliderFloat("FOV", &fFOVChanger, 60.f, 120.f, "%.0f");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Movement")) {
            ImGui::Spacing(); ImGui::Text("Movement Assistance"); ImGui::Separator();
            ImGui::Checkbox("Bunny Hop",     &bBhop);
            ImGui::Checkbox("Strafe Helper", &bStrafe);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Misc")) {
            ImGui::Spacing(); ImGui::Text("Overlay"); ImGui::Separator();
            ImGui::Checkbox("Radar",          &bRadar);
            ImGui::Checkbox("Bomb Timer",     &bBombTimer);
            ImGui::Checkbox("Spectator List", &bSpectatorList);
            ImGui::Spacing(); ImGui::Text("Settings"); ImGui::Separator();

            // FIXED: buttons now actually call Config functions
            if (ImGui::Button("Save Config", ImVec2(-1, 0)))
                Config::Save();

            if (ImGui::Button("Reset to Defaults", ImVec2(-1, 0)))
                Config::ResetDefaults();

            ImGui::Spacing(); ImGui::Separator();
            ImGui::TextDisabled("INSERT  - toggle menu");
            ImGui::TextDisabled("END     - unload cheat");
            ImGui::TextDisabled("v1.1 - LitWare CS2");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}
