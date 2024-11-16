#include "UIManager.h"
#include <string>
#include <iostream>
#include <cstring>

UIManager::UIManager(std::shared_ptr<WorldMap> worldMap)
    : initialized(false), renderWindow(nullptr), worldMap(worldMap), timeScalePtr(nullptr)
{
    // Initialize default color and thickness
    // These can be removed if not needed
}

UIManager::~UIManager() {
    Shutdown();
}

bool UIManager::Init() {
    if (renderWindow) {
        ImGui::SFML::Init(*renderWindow);
        initialized = true;
        return true;
    }
    return false;
}

void UIManager::SetWindow(sf::RenderWindow& window) {
    renderWindow = &window;
}

void UIManager::ProcessEvent(const sf::Event& event) {
    if (initialized) {
        ImGui::SFML::ProcessEvent(event);
    }
}

void UIManager::Update(float deltaTime) {
    if (initialized && renderWindow) {
        ImGui::SFML::Update(*renderWindow, sf::seconds(deltaTime));
    }
}

void UIManager::Render() {
    if (!initialized) return;

    RenderLineProperties();
    RenderStationProperties();

    ImGui::SFML::Render(*renderWindow);
}

void UIManager::Shutdown() {
    if (initialized) {
        ImGui::SFML::Shutdown();
        initialized = false;
    }
}

void UIManager::RegisterUIAction(const std::string& action, std::function<void()> callback) {
    uiActionCallbacks.emplace_back(action, callback);
}

void UIManager::EmitUIAction(const std::string& action) {
    for (const auto& pair : uiActionCallbacks) {
        if (pair.first == action) {
            pair.second();
            return;
        }
    }
}

void UIManager::SetTimeScalePointer(std::atomic<float>* ptr) {
    timeScalePtr = ptr;
}

void UIManager::RenderLineProperties() {
    Line* selectedLine = worldMap->GetSelectedLine();
    if (selectedLine) {
        ImGui::Begin("Line Properties");

        // Thickness slider
        static float thickness = selectedLine->GetThickness();
        if (ImGui::SliderFloat("Line Thickness", &thickness, 1.0f, 10.0f)) {
            selectedLine->SetThickness(thickness);
        }

        // Speed control
        static float speed = selectedLine->GetSpeed();
        if (ImGui::InputFloat("Speed (km/h)", &speed)) {
            selectedLine->SetSpeed(speed);
        }

        // Button to add a train
        if (ImGui::Button("Add Train")) {
            selectedLine->AddTrain();
        }

        // Button to remove all trains
        if (ImGui::Button("Remove All Trains")) {
            selectedLine->RemoveTrains();
        }

        ImGui::End();
    }
}

void UIManager::RenderStationProperties() {
    Station* selectedStation = worldMap->GetSelectedStation();
    if (selectedStation) {
        ImGui::Begin("Station Properties");

        // Get and edit station name
        static char nameBuffer[128];
        errno_t err = strncpy_s(nameBuffer, sizeof(nameBuffer), selectedStation->GetName().c_str(), _TRUNCATE);
        if (err != 0) {
            std::cerr << "Error copying station name with strncpy_s" << std::endl;
            nameBuffer[0] = '\0'; // Ensure buffer is null-terminated
        }

        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
            selectedStation->SetName(std::string(nameBuffer));
        }

        // Additional station properties can be added here

        ImGui::End();
    }
}
