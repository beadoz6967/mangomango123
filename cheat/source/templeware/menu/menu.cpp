#include "menu.h"
#include "../config/config.h"

#include "../utils/logging/log.h"

#include <iostream>
#include <vector>
#include "../config/configmanager.h"
#include <filesystem>

#include "../keybinds/keybinds.h"

#include "../utils/logging/log.h"

void ApplyImGuiTheme() {
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    ImVec4 primaryColor = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
    ImVec4 outlineColor = ImVec4(0.0f, 0.5f, 0.8f, 0.7f);
    ImVec4 bgColor = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
    ImVec4 panelColor = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);

    colors[ImGuiCol_WindowBg] = bgColor;
    colors[ImGuiCol_Border] = outlineColor;
    colors[ImGuiCol_FrameBg] = panelColor;
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_TitleBg] = bgColor;
    colors[ImGuiCol_TitleBgActive] = bgColor;
    colors[ImGuiCol_PopupBg] = bgColor;

    colors[ImGuiCol_Button] = primaryColor;
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.2f, 0.9f, 1.0f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.7f, 0.9f, 1.0f);

    colors[ImGuiCol_CheckMark] = primaryColor;
    colors[ImGuiCol_SliderGrab] = primaryColor;
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.2f, 0.9f, 1.0f, 1.0f);

    colors[ImGuiCol_Header] = primaryColor;
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.2f, 0.9f, 1.0f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.7f, 0.9f, 1.0f);

    colors[ImGuiCol_Separator] = outlineColor;
    colors[ImGuiCol_SeparatorHovered] = primaryColor;
    colors[ImGuiCol_SeparatorActive] = primaryColor;

    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);

    colors[ImGuiCol_Tab] = panelColor;
    colors[ImGuiCol_TabHovered] = ImVec4(0.0f, 0.5f, 0.8f, 0.8f);
    colors[ImGuiCol_TabActive] = primaryColor;
    colors[ImGuiCol_TabUnfocused] = panelColor;
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0f, 0.4f, 0.7f, 1.0f);

    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.ChildRounding = 6.0f;
    style.PopupRounding = 4.0f;

    style.ItemSpacing = ImVec2(10, 6);
    style.FramePadding = ImVec2(6, 4);
    style.WindowPadding = ImVec2(10, 10);

    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    style.WindowBorderSize = 1.0f;
    style.PopupBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
}

Menu::Menu() {
    activeTab = 0;
    showMenu = true;
}

void Menu::init(HWND& window, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11RenderTargetView* mainRenderTargetView) {
    if (initialized) {
        Logger::Log("Menu already initialized; skipping duplicate setup.", LogType::Warning);
        return;
    }

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(pDevice, pContext);

    ApplyImGuiTheme();

    io.Fonts->AddFontDefault();

    const std::filesystem::path arialPath = R"(C:\Windows\Fonts\arial.ttf)";
    if (std::filesystem::exists(arialPath)) {
        if (io.Fonts->AddFontFromFileTTF(arialPath.string().c_str(), 16.0f) == nullptr) {
            Logger::Log("Menu font fallback: Arial failed to load, using default font only.", LogType::Warning);
        }
    } else {
        Logger::Log("Menu font fallback: Arial not found, using default font only.", LogType::Warning);
    }

    io.Fonts->Build();
    initialized = true;

    Logger::Log("Menu initialized.", LogType::Info);
}

void Menu::render() {
    keybind.pollInputs();
    if (showMenu) {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar;

        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);

        ImGui::Begin("TempleWare | Internal", nullptr, window_flags);

        {
            float windowWidth = ImGui::GetWindowWidth();
            float rightTextWidth = ImGui::CalcTextSize("templecheats.xyz").x;

            ImGui::Text("TempleWare - Internal");

            ImGui::SameLine(windowWidth - rightTextWidth - 10);
            ImGui::Text("templecheats.xyz");
        }

        ImGui::Separator();

        const char* tabNames[] = { "Aim", "Visuals", "Misc", "Config" };

        if (ImGui::BeginTabBar("MainTabBar", ImGuiTabBarFlags_NoTooltip)) {
            for (int i = 0; i < 4; i++) {
                if (ImGui::BeginTabItem(tabNames[i])) {
                    activeTab = i;
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::BeginChild("ContentRegion", ImVec2(0, 0), false);

        switch (activeTab) {
        case 0:
        {
            ImGui::BeginChild("AimLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 5, 0), true);
            ImGui::Text("General");
            ImGui::Separator();

            ImGui::Checkbox("Enable##AimBot", &Config::aimbot);
            ImGui::SameLine();
            ImGui::Text("Key:");
            ImGui::SameLine();
            keybind.menuButton(Config::aimbot);

            ImGui::Checkbox("Team Check", &Config::team_check);
            ImGui::SliderFloat("FOV", &Config::aimbot_fov, 0.f, 90.f);
            ImGui::Checkbox("Draw FOV Circle", &Config::fov_circle);
            if (Config::fov_circle) {
                ImGui::ColorEdit4("Circle Color##FovColor", (float*)&Config::fovCircleColor);
            }
            ImGui::Checkbox("Recoil Control", &Config::rcs);
            ImGui::EndChild();

            ImGui::SameLine();
            ImGui::BeginChild("AimRight", ImVec2(0, 0), true);
            ImGui::Text("TriggerBot");
            ImGui::Separator();
            ImGui::Text("No additional settings");

            ImGui::EndChild();
        }
        break;

        case 1:
        {
            ImGui::BeginChild("VisualsLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 5, 0), true);
            ImGui::Text("Player ESP");
            ImGui::Separator();

            ImGui::Checkbox("Box", &Config::esp);
            ImGui::SliderFloat("Thickness", &Config::espThickness, 1.0f, 5.0f);
            ImGui::Checkbox("Box Fill", &Config::espFill);
            if (Config::espFill) {
                ImGui::SliderFloat("Fill Opacity", &Config::espFillOpacity, 0.0f, 1.0f);
            }
            ImGui::ColorEdit4("ESP Color##BoxColor", (float*)&Config::espColor);
            ImGui::Checkbox("Team Check", &Config::teamCheck);
            ImGui::Checkbox("Health Bar", &Config::showHealth);
            ImGui::Checkbox("Name Tags", &Config::showNameTags);

            ImGui::Spacing();
            ImGui::Text("World");
            ImGui::Separator();

            ImGui::Checkbox("Night Mode", &Config::Night);
            if (Config::Night) {
                ImGui::ColorEdit4("Night Color", (float*)&Config::NightColor);
            }

            ImGui::Checkbox("Custom FOV", &Config::fovEnabled);
            if (Config::fovEnabled) {
                ImGui::SliderFloat("FOV Value##FovSlider", &Config::fov, 20.0f, 160.0f, "%1.0f");
            }

            ImGui::EndChild();

            ImGui::SameLine();
            ImGui::BeginChild("VisualsRight", ImVec2(0, 0), true);
            ImGui::Text("Chams");
            ImGui::Separator();

            ImGui::Checkbox("Chams##ChamsCheckbox", &Config::enemyChams);
            const char* chamsMaterials[] = { "Flat", "Illuminate", "Glow" };
            ImGui::Combo("Material", &Config::chamsMaterial, chamsMaterials, IM_ARRAYSIZE(chamsMaterials));
            if (Config::enemyChams) {
                ImGui::ColorEdit4("Chams Color##ChamsColor", (float*)&Config::colVisualChams);
            }
            ImGui::Checkbox("Chams-XQZ", &Config::enemyChamsInvisible);
            if (Config::enemyChamsInvisible) {
                ImGui::ColorEdit4("XQZ Color##ChamsXQZColor", (float*)&Config::colVisualChamsIgnoreZ);
            }

            ImGui::Spacing();
            ImGui::Text("Hand Chams");
            ImGui::Separator();

            ImGui::Checkbox("Hand Chams", &Config::armChams);
            if (Config::armChams) {
                ImGui::ColorEdit4("Hand Color##HandChamsColor", (float*)&Config::colArmChams);
            }
            ImGui::Checkbox("Viewmodel Chams", &Config::viewmodelChams);
            if (Config::viewmodelChams) {
                ImGui::ColorEdit4("Viewmodel Color##ViewModelChamsColor", (float*)&Config::colViewmodelChams);
            }

            ImGui::Spacing();
            ImGui::Text("Removals");
            ImGui::Separator();

            ImGui::Checkbox("Anti Flash", &Config::antiflash);

            ImGui::EndChild();
        }
        break;

        case 2:
        {
            ImGui::BeginChild("MiscLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 5, 0), true);
            ImGui::Text("Movement");
            ImGui::Separator();

            ImGui::Text("No additional settings");

            ImGui::EndChild();

            ImGui::SameLine();
            ImGui::BeginChild("MiscRight", ImVec2(0, 0), true);
            ImGui::Text("Other");
            ImGui::Separator();

            ImGui::Text("No additional settings");

            ImGui::EndChild();
        }
        break;

        case 3:
        {
            ImGui::BeginChild("ConfigLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 5, 0), true);
            ImGui::Text("General");
            ImGui::Separator();

            static char configName[128] = "";
            static std::vector<std::string> configList = internal_config::ConfigManager::ListConfigs();
            static int selectedConfigIndex = -1;

            ImGui::InputText("Config Name", configName, IM_ARRAYSIZE(configName));

            if (ImGui::Button("Refresh")) {
                configList = internal_config::ConfigManager::ListConfigs();
            }
            ImGui::SameLine();
            if (ImGui::Button("Load")) {
                internal_config::ConfigManager::Load(configName);
            }
            ImGui::SameLine();
            if (ImGui::Button("Save")) {
                internal_config::ConfigManager::Save(configName);
                configList = internal_config::ConfigManager::ListConfigs();
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                internal_config::ConfigManager::Remove(configName);
                configList = internal_config::ConfigManager::ListConfigs();
            }

            ImGui::EndChild();

            ImGui::SameLine();
            ImGui::BeginChild("ConfigRight", ImVec2(0, 0), true);
            ImGui::Text("Saved Configs");
            ImGui::Separator();

            for (int i = 0; i < static_cast<int>(configList.size()); i++) {
                if (ImGui::Selectable(configList[i].c_str(), selectedConfigIndex == i)) {
                    selectedConfigIndex = i;
                    strncpy_s(configName, sizeof(configName), configList[i].c_str(), _TRUNCATE);
                }
            }

            ImGui::EndChild();
        }
        break;
        }

        ImGui::EndChild();
        ImGui::End();
    }
}

void Menu::toggleMenu() {
    showMenu = !showMenu;
}