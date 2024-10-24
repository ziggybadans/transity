#include "UIManager.h"

UIManager::UIManager()
    : initialized(false), renderWindow(nullptr) {}

UIManager::~UIManager() {
    Shutdown();
}

void UIManager::SetWindow(sf::RenderWindow& window) {
    renderWindow = &window;
}

bool UIManager::Init() {
    if (renderWindow) { // Ensure the window is set
        ImGui::SFML::Init(*renderWindow); // Initialize ImGui-SFML
        initialized = true;
        return true;
    }
    return false;
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
    if (initialized && renderWindow) {
        ImGui::SFML::Render(*renderWindow);
    }
}

void UIManager::Shutdown() {
    if (initialized) {
        ImGui::SFML::Shutdown();
        initialized = false;
    }
}
