#include "UIManager.h"
#include <string>
#include <iostream>

UIManager::UIManager(std::shared_ptr<WorldMap> worldMap)
    : m_initialized(false)
    , m_renderWindow(nullptr)
    , m_worldMap(worldMap)
    , m_timeScalePtr(nullptr)
{
}

UIManager::~UIManager() {
    Shutdown();
}

bool UIManager::Init() {
    if (m_renderWindow) {
        if (!ImGui::SFML::Init(*m_renderWindow)) {
            std::cerr << "Failed to initialize ImGui SFML." << std::endl;
            return false;
        }
        m_initialized = true;
        return true;
    }
    return false;
}

void UIManager::ProcessEvent(const sf::Event& event) {
    if (m_initialized) {
        ImGui::SFML::ProcessEvent(event);
    }
}

void UIManager::Update(float deltaTime) {
    if (m_initialized && m_renderWindow) {
        ImGui::SFML::Update(*m_renderWindow, sf::seconds(deltaTime));
    }
}

void UIManager::Render() {
    if (!m_initialized) return;

    ImGui::SFML::Render(*m_renderWindow);
}

void UIManager::Shutdown() {
    if (m_initialized) {
        ImGui::SFML::Shutdown();
        m_initialized = false;
    }
}

void UIManager::SetWindow(sf::RenderWindow& window) {
    m_renderWindow = &window;
}
