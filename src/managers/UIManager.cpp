#include "UIManager.h"
#include <iostream>
#include <array>
#include "../Debug.h"

UIManager::UIManager()
    : m_initialized(false)
    , m_renderWindow(nullptr)
    , m_timeScalePtr(nullptr)
    , m_fps(0.0f)
    , m_showSettingsPanel(false)
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
        
        // Settings button in top-right corner
        ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_renderWindow->getSize().x) - 100.0f, 10.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(90.0f, 40.0f), ImGuiCond_Always);
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
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("FPS: %.1f", m_fps);
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

    sf::Vector2u currentRes = m_gameSettings->GetResolution();
    int currentItem = -1;
    
    // Find current resolution in the list
    for (size_t i = 0; i < resolutions.size(); i++) {
        if (resolutions[i] == currentRes) {
            currentItem = static_cast<int>(i);
            break;
        }
    }

    // Create items string array for ImGui::Combo
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
            m_gameSettings->SetResolution(resolutions[currentItem]);
            settingsChanged = true;
        }
    }

    bool fullscreen = m_gameSettings->IsFullscreen();
    if (ImGui::Checkbox("Fullscreen", &fullscreen)) {
        m_gameSettings->SetFullscreen(fullscreen);
        settingsChanged = true;
    }

    bool vsync = m_gameSettings->IsVSyncEnabled();
    if (ImGui::Checkbox("V-Sync", &vsync)) {
        m_gameSettings->SetVSync(vsync);
        m_windowManager->SetVSync(vsync);
    }

    int frameLimit = static_cast<int>(m_gameSettings->GetFrameRateLimit());
    if (ImGui::SliderInt("Frame Rate Limit", &frameLimit, 30, 240)) {
        m_gameSettings->SetFrameRateLimit(frameLimit);
        m_windowManager->SetFramerateLimit(frameLimit);
    }

    if (settingsChanged && m_windowManager) {
        m_windowManager->SetVideoMode(sf::VideoMode(m_gameSettings->GetResolution().x, m_gameSettings->GetResolution().y));
        m_windowManager->SetFullscreen(m_gameSettings->IsFullscreen());
        m_windowManager->ApplyVideoMode();
    }
}

void UIManager::RenderGameplaySettings() {
    float zoomSpeed = m_gameSettings->GetCameraZoomSpeed();
    if (ImGui::SliderFloat("Camera Zoom Speed", &zoomSpeed, 1.0f, 2.0f, "%.2f")) {
        m_gameSettings->SetCameraZoomSpeed(zoomSpeed);
        if (m_inputManager) {
            m_inputManager->SetZoomSpeed(zoomSpeed);
        }
    }

    float panSpeed = m_gameSettings->GetCameraPanSpeed();
    if (ImGui::SliderFloat("Camera Pan Speed", &panSpeed, 100.0f, 1000.0f, "%.0f")) {
        m_gameSettings->SetCameraPanSpeed(panSpeed);
        if (m_inputManager) {
            m_inputManager->SetPanSpeed(panSpeed);
        }
    }

    int autosaveInterval = static_cast<int>(m_gameSettings->GetAutosaveInterval());
    if (ImGui::SliderInt("Autosave Interval (minutes)", &autosaveInterval, 1, 30)) {
        m_gameSettings->SetAutosaveInterval(autosaveInterval);
    }
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
