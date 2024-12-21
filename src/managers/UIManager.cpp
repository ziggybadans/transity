#include "UIManager.h"
#include <iostream>
#include <array>
#include "../Debug.h"
#include "../utility/Profiler.h"
#include "../settings/SettingsDefinitions.h"

UIManager::UIManager()
    : m_initialized(false)
    , m_renderWindow(nullptr)
    , m_timeScalePtr(nullptr)
    , m_fps(0.0f)
    , m_showSettingsPanel(false)
    , m_showPerformanceWindow(true)
    , m_gameSettings(nullptr)
{}

UIManager::~UIManager() {
    Shutdown();
}

bool UIManager::Init() {
    if (!m_renderWindow) {
        DEBUG_ERROR("No render window set for UIManager");
        return false;
    }

    try {
        if (!ImGui::SFML::Init(*m_renderWindow)) {
            DEBUG_ERROR("Failed to initialize ImGui SFML.");
            return false;
        }
        m_initialized = true;
        return true;
    } catch (const std::exception& e) {
        DEBUG_ERROR("Exception during ImGui initialization: " + std::string(e.what()));
        return false;
    }
}

void UIManager::ProcessEvent(const sf::Event& event) {
    if (m_initialized && m_renderWindow) {
        ImGui::SFML::ProcessEvent(event);
    }
}

void UIManager::Update(float deltaTime) {
    if (!m_initialized || !m_renderWindow) {
        return;
    }

    try {
        ImGui::SFML::Update(*m_renderWindow, sf::seconds(deltaTime));
        m_fps = deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;
    } catch (const std::exception& e) {
        DEBUG_ERROR("Error updating UI: " + std::string(e.what()));
        Shutdown(); // Safely shutdown on error
    }
}

void UIManager::Render() {
    if (!m_initialized || !m_renderWindow) {
        return;
    }

    try {
        RenderPerformanceOverlay();
        RenderPerformanceWindow();
        RenderGUI();
        
        // Settings button in top-right corner
        ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_renderWindow->getSize().x) - 140.0f, 10.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(98.0f, 48.0f), ImGuiCond_Always);
        if (ImGui::Begin("Settings Button", nullptr, 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            if (ImGui::Button("Settings", ImVec2(80, 30))) {
                m_showSettingsPanel = !m_showSettingsPanel;
            }
        }
        ImGui::End();

        if (m_showSettingsPanel) {
            RenderSettingsPanel();
        }

        ImGui::SFML::Render(*m_renderWindow);
    } catch (const std::exception& e) {
        DEBUG_ERROR("Error rendering UI: " + std::string(e.what()));
        Shutdown();
    }
}

void UIManager::RenderPerformanceOverlay() {
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(200.0f, 60.0f), ImGuiCond_Always);
    ImGui::Begin("Performance", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("FPS: %.1f", m_fps);
    if (ImGui::Button(m_showPerformanceWindow ? "Hide Profiler" : "Show Profiler")) {
        m_showPerformanceWindow = !m_showPerformanceWindow;
    }
    ImGui::End();
}

void UIManager::RenderSettingsPanel() {
    if (!m_gameSettings) return;

    ImGui::SetNextWindowPos(
        ImVec2(
            static_cast<float>(m_renderWindow->getSize().x) / 2.0f - 300.0f,
            static_cast<float>(m_renderWindow->getSize().y) / 2.0f - 200.0f
        ),
        ImGuiCond_FirstUseEver
    );
    ImGui::SetNextWindowSize(ImVec2(600.0f, 400.0f), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Settings", &m_showSettingsPanel)) {
        if (ImGui::BeginTabBar("SettingsTabs")) {
            if (ImGui::BeginTabItem("Video")) {
                RenderVideoSettings();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Gameplay")) {
                RenderGameplaySettings();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::Separator();

        if (ImGui::Button("Save Settings")) {
            m_gameSettings->SaveSettings("config/settings.json");
        }
        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            m_showSettingsPanel = false;
        }
    }
    ImGui::End();
}

void UIManager::RenderVideoSettings() {
    bool settingsChanged = false;
    
    static const std::array<sf::Vector2u, 4> resolutions = {
        sf::Vector2u(1920, 1080),
        sf::Vector2u(2560, 1440),
        sf::Vector2u(3440, 1440),
        sf::Vector2u(3840, 2160)
    };

    sf::Vector2u currentRes = m_gameSettings->GetValue<sf::Vector2u>(Settings::Names::RESOLUTION);
    int currentItem = -1;
    
    // Find current resolution in the list
    for (size_t i = 0; i < resolutions.size(); i++) {
        if (resolutions[i] == currentRes) {
            currentItem = static_cast<int>(i);
            break;
        }
    }

    std::vector<std::string> items;
    for (const auto& res : resolutions) {
        items.push_back(std::to_string(res.x) + "x" + std::to_string(res.y));
    }

    std::vector<const char*> itemsStr;
    for (const auto& item : items) {
        itemsStr.push_back(item.c_str());
    }

    if (ImGui::Combo("Resolution", &currentItem, itemsStr.data(), static_cast<int>(itemsStr.size()))) {
        if (currentItem >= 0 && currentItem < static_cast<int>(resolutions.size())) {
            m_gameSettings->SetValue(Settings::Names::RESOLUTION, resolutions[currentItem]);
            settingsChanged = true;
        }
    }

    bool fullscreen = m_gameSettings->GetValue<bool>(Settings::Names::FULLSCREEN);
    if (ImGui::Checkbox("Fullscreen", &fullscreen)) {
        m_gameSettings->SetValue(Settings::Names::FULLSCREEN, fullscreen);
        settingsChanged = true;
    }

    bool vsync = m_gameSettings->GetValue<bool>(Settings::Names::VSYNC);
    if (ImGui::Checkbox("V-Sync", &vsync)) {
        m_gameSettings->SetValue(Settings::Names::VSYNC, vsync);
        m_windowManager->SetVSync(vsync);
    }

    int frameLimit = static_cast<int>(m_gameSettings->GetValue<unsigned int>(Settings::Names::FRAME_RATE_LIMIT));
    if (ImGui::SliderInt("Frame Rate Limit", &frameLimit, 30, 240)) {
        m_gameSettings->SetValue(Settings::Names::FRAME_RATE_LIMIT, static_cast<unsigned int>(frameLimit));
        m_windowManager->SetFramerateLimit(frameLimit);
    }

    if (settingsChanged && m_windowManager) {
        auto resolution = m_gameSettings->GetValue<sf::Vector2u>(Settings::Names::RESOLUTION);
        m_windowManager->SetVideoMode(sf::VideoMode(resolution.x, resolution.y));
        m_windowManager->SetFullscreen(m_gameSettings->GetValue<bool>(Settings::Names::FULLSCREEN));
        m_windowManager->ApplyVideoMode();
    }
}

void UIManager::RenderGameplaySettings() {
    float zoomSpeed = m_gameSettings->GetValue<float>(Settings::Names::CAMERA_ZOOM_SPEED);
    if (ImGui::SliderFloat("Camera Zoom Speed", &zoomSpeed, 1.0f, 2.0f, "%.2f")) {
        m_gameSettings->SetValue(Settings::Names::CAMERA_ZOOM_SPEED, zoomSpeed);
    }

    float panSpeed = m_gameSettings->GetValue<float>(Settings::Names::CAMERA_PAN_SPEED);
    if (ImGui::SliderFloat("Camera Pan Speed", &panSpeed, 100.0f, 1000.0f, "%.0f")) {
        m_gameSettings->SetValue(Settings::Names::CAMERA_PAN_SPEED, panSpeed);
    }

    int autosaveInterval = static_cast<int>(m_gameSettings->GetValue<unsigned int>(Settings::Names::AUTOSAVE_INTERVAL));
    if (ImGui::SliderInt("Autosave Interval (minutes)", &autosaveInterval, 1, 30)) {
        m_gameSettings->SetValue(Settings::Names::AUTOSAVE_INTERVAL, static_cast<unsigned int>(autosaveInterval));
    }
}

void UIManager::RenderPerformanceWindow() {
    if (!m_showPerformanceWindow) return;

    ImGui::SetNextWindowPos(ImVec2(10.0f, 80.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300.0f, 400.0f), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Performance Profiler", &m_showPerformanceWindow)) {
        ImGui::Text("FPS: %.1f", m_fps);
        ImGui::Separator();
        
        if (ImGui::BeginTable("ProfilerData", 2, 
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
            
            ImGui::TableSetupColumn("Section", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Time (ms)", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();

            auto profiles = Profiler::GetSortedProfiles();
            for (const auto& profile : profiles) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", profile.name.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%.3f", profile.duration);
            }
            
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void UIManager::RenderGUI()
{
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_renderWindow->getSize().x) / 2, (m_renderWindow->getSize().y) - 100.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(98.0f, 48.0f), ImGuiCond_Always);
    if (ImGui::Begin("Tools", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {
        if (ImGui::Button("Line Mode", ImVec2(80, 30))) {
            m_eventManager->Dispatch(EventType::ToolChanged, ToolChangedEvent{ "LineTool" });
        }
    }
    ImGui::End();
}

void UIManager::Shutdown() {
    if (m_initialized) {
        try {
            ImGui::SFML::Shutdown();
        } catch (const std::exception& e) {
            DEBUG_ERROR("Error during ImGui shutdown: " + std::string(e.what()));
        }
        m_initialized = false;
    }
    m_renderWindow = nullptr; // Clear the window pointer
}

void UIManager::SetWindow(sf::RenderWindow& window) {
    if (!window.isOpen()) {
        throw std::invalid_argument("Cannot set closed window");
    }
    
    // If we already have an initialized ImGui context, shut it down first
    if (m_initialized) {
        Shutdown();
    }
    
    m_renderWindow = &window;
}
