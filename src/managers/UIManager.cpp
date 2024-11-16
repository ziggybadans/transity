#include "UIManager.h"
#include <string>
#include <iostream>

UIManager::UIManager(std::shared_ptr<WorldMap> worldMap)
    : initialized(false), renderWindow(nullptr), worldMap(worldMap), timeScalePtr(nullptr)
{
    // Initialize default settings if needed
}

UIManager::~UIManager() {
    Shutdown();
}

bool UIManager::Init() {
    if (renderWindow) {
        if (!ImGui::SFML::Init(*renderWindow)) {
            std::cerr << "Failed to initialize ImGui SFML." << std::endl;
            return false;
        }
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

    // Example UI: Display information about place areas
    RenderPlaceAreaInfo();

    ImGui::SFML::Render(*renderWindow);
}

void UIManager::Shutdown() {
    if (initialized) {
        ImGui::SFML::Shutdown();
        initialized = false;
    }
}

void UIManager::RenderPlaceAreaInfo() {
    ImGui::Begin("Place Areas");

    const auto& placeAreas = worldMap->GetPlaceAreas();
    for (const auto& area : placeAreas) {
        ImGui::Text("Name: %s", area.name.c_str());
        ImGui::Text("Category: %d", static_cast<int>(area.category));
        ImGui::Separator();
    }

    ImGui::End();
}
