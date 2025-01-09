// UIManager.cpp
#include "UIManager.h"
#include <iostream>
#include <array>
#include "../Debug.h"
#include "../utility/Profiler.h"
#include "../settings/SettingsDefinitions.h"
#include "../world/Map.h"
#include "../entity/Passenger.h"

UIManager::UIManager()
    : m_initialized(false)
    , m_renderWindow(nullptr)
    , m_fps(0.0f)
    , m_showSettingsPanel(false)
    , m_showPerformanceWindow(true)
    , m_gameSettings(nullptr)
    , m_setTimeScale(nullptr)
    , m_getTimeScale(nullptr)
{
}

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
    }
    catch (const std::exception& e) {
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
    }
    catch (const std::exception& e) {
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
        RenderInfoPanel();
        RenderTimeControls();
        RenderGUI();

        // Settings button in top-right corner
        ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_renderWindow->getSize().x) - 110.0f, 10.0f), ImGuiCond_Always);
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
    }
    catch (const std::exception& e) {
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

void UIManager::RenderInfoPanel() {
    if (!m_map) {
        DEBUG_ERROR("UIManager::RenderInfoPanel - Map is not set.");
        return;
    }

    Train* selectedTrain = m_map->GetSelectedTrain();
    Line* selectedLine = m_map->GetSelectedLine();
    City* selectedCity = m_map->GetSelectedCity();
    if (!selectedTrain && !selectedLine && !selectedCity) {
        // No train or line is selected; do not render the panel
        return;
    }

    if (selectedTrain) {
        // Define panel size
        const float panelWidth = 300.0f;
        const float panelHeight = 220.0f; // Increased height to accommodate extra info

        // Position window at bottom-right corner with padding
        ImGui::SetNextWindowPos(ImVec2(
            static_cast<float>(m_renderWindow->getSize().x) - panelWidth - 10.0f,
            static_cast<float>(m_renderWindow->getSize().y) - panelHeight - 10.0f
        ), ImGuiCond_Always);

        ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);
        ImGui::Begin("Train Information", nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImGui::Text("Train Details");
        ImGui::Separator();

        // Route Name
        Line* route = selectedTrain->GetRoute();
        if (route) {
            ImGui::Text("Route: %s", route->GetName().c_str());
        }
        else {
            ImGui::Text("Route: N/A");
        }

        // Position
        sf::Vector2f position = selectedTrain->GetPosition();
        ImGui::Text("Position: (%.2f, %.2f)", position.x, position.y);

        // Speed
        float currentSpeed = selectedTrain->GetSpeed();
        float maxSpeed = selectedTrain->GetMaxSpeed();
        ImGui::Text("Speed: %.2f / %.2f px/s", currentSpeed, maxSpeed);

        // State
        std::string state = selectedTrain->GetState();
        ImGui::Text("State: %s", state.c_str());

        // Direction
        std::string direction = selectedTrain->GetDirection();
        ImGui::Text("Direction: %s", direction.c_str());

        // Capacity and Passengers
        ImGui::Text("Capacity: %d", selectedTrain->GetCapacity());
        ImGui::Text("Passengers: %d", selectedTrain->GetPassengerCount());

        // Wait Time if applicable
        if (state == "Waiting") {
            float waitTime = selectedTrain->GetWaitTime();
            ImGui::Text("Wait Time: %.2f s", waitTime);
        }

        // New Passenger Table
        const auto& passengers = selectedTrain->GetPassengers();
        if (!passengers.empty()) {
            ImGui::Separator();
            ImGui::Text("Current Passengers:");
            if (ImGui::BeginTable("PassengerTable", 3,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 30.0f);
                ImGui::TableSetupColumn("Origin", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Destination", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (size_t i = 0; i < passengers.size(); ++i) {
                    Passenger* p = passengers[i];
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%zu", i + 1);

                    ImGui::TableNextColumn();
                    // Assuming GetOrigin() returns a pointer to the origin city
                    City* origin = p->GetOrigin();
                    ImGui::Text("%s", origin ? origin->GetName().c_str() : "Unknown");

                    ImGui::TableNextColumn();
                    // Assuming GetDestination() returns a pointer to the destination city
                    City* destination = p->GetDestination();
                    ImGui::Text("%s", destination ? destination->GetName().c_str() : "Unknown");
                }
                ImGui::EndTable();
            }
        }

        // Remove train button
        if (ImGui::Button("Remove", ImVec2(70, 30))) {
            m_map->RemoveTrain();
        }

        ImGui::End();
    }

    if (selectedLine) {
        // Define panel size
        const float panelWidth = 300.0f;
        const float panelHeight = 260.0f; // Increased height for extra info

        // Position window at bottom-right corner with padding
        ImGui::SetNextWindowPos(ImVec2(
            static_cast<float>(m_renderWindow->getSize().x) - panelWidth - 10.0f,
            static_cast<float>(m_renderWindow->getSize().y) - panelHeight - 10.0f
        ), ImGuiCond_Always);

        ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);
        ImGui::Begin("Line Information", nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Line Details");
        ImGui::Separator();

        // Display basic line properties
        ImGui::Text("Route: %s", selectedLine->GetName().c_str());
        sf::Color color = selectedLine->GetColor();
        ImGui::Text("Color: (%d, %d, %d)", color.r, color.g, color.b);
        ImGui::Text("Thickness: %.2f", selectedLine->GetThickness());
        ImGui::Separator();

        // Cities Table
        ImGui::Text("Cities on Line:");
        if (ImGui::BeginTable("CitiesTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("City Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Population", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableHeadersRow();

            const std::vector<City*> cities = selectedLine->GetCities();
            for (const auto& city : cities) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", city->GetName().c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%u", city->GetPopulation());
            }
            ImGui::EndTable();
        }
        ImGui::Separator();

        // Trains Table
        ImGui::Text("Trains on Line:");
        const std::vector<Train*> trains = selectedLine->GetTrains();
        if (trains.empty()) {
            ImGui::Text("No trains on this line.");
        }
        else {
            if (ImGui::BeginTable("TrainsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("Train ID", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Speed (px/s)", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (const auto& train : trains) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", train->GetID().c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%.2f", train->GetSpeed());
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", train->GetState().c_str());
                }

                ImGui::EndTable();
            }
        }
        ImGui::Separator();

        if (ImGui::Button("Remove", ImVec2(70, 30))) {
            m_map->RemoveLine();
        }

        ImGui::End();
    }

    if (selectedCity) {
        // Increase panel height to accommodate the passenger table
        const float panelWidth = 300.0f;
        const float panelHeight = 300.0f;

        ImGui::SetNextWindowPos(ImVec2(
            static_cast<float>(m_renderWindow->getSize().x) - panelWidth - 10.0f,
            static_cast<float>(m_renderWindow->getSize().y) - panelHeight - 10.0f
        ), ImGuiCond_Always);

        ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

        ImGui::Begin("City Information", nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::Text("City: %s", selectedCity->GetName().c_str());

        // New Passenger Table for Waiting Passengers at the City
        const auto& waitingPassengers = selectedCity->GetWaitingPassengers();
        if (!waitingPassengers.empty()) {
            ImGui::Separator();
            ImGui::Text("Waiting Passengers:");
            if (ImGui::BeginTable("CityPassengerTable", 2,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 30.0f);
                ImGui::TableSetupColumn("Destination", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (size_t i = 0; i < waitingPassengers.size(); ++i) {
                    Passenger* p = waitingPassengers[i];
                    City* destination = p->GetDestination();

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%zu", i + 1);

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", destination ? destination->GetName().c_str() : "Unknown");
                }

                ImGui::EndTable();
            }
        }

        if (ImGui::Button("Remove", ImVec2(70, 30))) {
            m_map->RemoveCity(selectedCity);
        }

        ImGui::End();
    }

}

void UIManager::RenderTimeControls()
{
    // Position the time controls window at the bottom-left corner
    ImGui::SetNextWindowPos(ImVec2(10.0f, static_cast<float>(m_renderWindow->getSize().y) - 110.0f), ImGuiCond_Always);
    //ImGui::SetNextWindowSize(ImVec2(200.0f, 90.0f), ImGuiCond_Always);

    // Create a window for Time Controls without title bar and with no resizing/moving
    if (ImGui::Begin("Time Controls", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize))
    {
        float currentScale = m_getTimeScale();
        bool isPaused = (currentScale == 0.0f);

        // Display current time scale
        ImGui::Text("Time Scale: %.2fx", isPaused ? 0.0f : currentScale);
        ImGui::Separator();

        bool canSpeedUp = (currentScale < 4.0f);
        bool canSlowDown = (currentScale > 0.25f);

        // Speed Up Button
        if (!canSpeedUp) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Gray out
        }
        if (ImGui::Button("Speed Up", ImVec2(70, 30)))
        {
            if (canSpeedUp && !isPaused)
            {
                float newScale = std::min(currentScale + 0.25f, 4.0f);
                m_setTimeScale(newScale);
            }
        }
        if (!canSpeedUp) {
            ImGui::PopStyleColor();
        }
        ImGui::SameLine();

        // Slow Down Button
        if (!canSlowDown) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Gray out
        }
        if (ImGui::Button("Slow Down", ImVec2(70, 30)))
        {
            if (canSlowDown && !isPaused)
            {
                float newScale = std::max(currentScale - 0.25f, 0.25f);
                m_setTimeScale(newScale);
            }
        }
        if (!canSlowDown) {
            ImGui::PopStyleColor();
        }

        // Pause/Resume Button
        if (isPaused)
        {
            if (ImGui::Button("Resume", ImVec2(148, 30)))
            {
                m_setTimeScale(m_lastTimeScale);
            }
        }
        else
        {
            if (ImGui::Button("Pause", ImVec2(148, 30)))
            {
                m_lastTimeScale = currentScale; // Save current scale before pausing
                m_setTimeScale(0.0f);
            }
        }
    }
    ImGui::End();
}

void UIManager::RenderGUI()
{
    static bool isLineMode = false;
    static bool isTrainMode = false;

    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_renderWindow->getSize().x) / 2 - 100.0f, (m_renderWindow->getSize().y) - 100.0f), ImGuiCond_Always);

    if (ImGui::Begin("Tools", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::BeginDisabled(isTrainMode);
            if (ImGui::Button(isLineMode ? "Line Mode ON" : "Line Mode OFF", ImVec2(95, 30))) {
                isLineMode = !isLineMode;

                if (isLineMode) {
                    m_stateManager->SetState("CurrentTool", std::string("Line"));
                }
                else {
                    m_stateManager->SetState("CurrentTool", std::string("Place"));
                }
            }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(isLineMode);
            if (ImGui::Button(isTrainMode ? "Train Mode ON" : "Train Mode OFF", ImVec2(100, 30))) {
                isTrainMode = !isTrainMode;

                if (isTrainMode) {
                    DEBUG_DEBUG("Train mode is currently false. Setting to true...");
                    m_stateManager->SetState("CurrentTool", std::string("TrainPlace"));
                }
                else {
                    DEBUG_DEBUG("Train mode is currently true. Setting to false...");
                    m_stateManager->SetState("CurrentTool", std::string("Place"));
                }

                m_map->DeselectAll();
            }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(!m_stateManager->GetState<bool>("TrainPlaceVerified"));
            if (ImGui::Button("Add Train", ImVec2(95, 30))) {
                m_map->AddTrain();
                m_stateManager->SetState("TrainPlaceVerified", false);
            }
        ImGui::EndDisabled();
;    }
    ImGui::End();
}

void UIManager::Shutdown() {
    if (m_initialized) {
        try {
            ImGui::SFML::Shutdown();
        }
        catch (const std::exception& e) {
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
