#include "UIManager.h"
#include <string>
#include <iostream>

UIManager::UIManager(std::shared_ptr<WorldMap> worldMap)
    : m_initialized(false)
    , m_renderWindow(nullptr)
    , m_worldMap(std::move(worldMap))
    , m_timeScalePtr(nullptr)
    , m_fps(0.0f)
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
    if (!m_initialized || !m_renderWindow) {
        return;
    }

    try {
        ImGui::SFML::Update(*m_renderWindow, sf::seconds(deltaTime));
        m_fps = deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;
    } catch (const std::exception& e) {
        std::cerr << "Error updating UI: " << e.what() << std::endl;
    }
}

void UIManager::Render() {
    if (!m_initialized) return;

    ImGui::Begin("Performance");

    ImGui::Text("FPS: %.1f", m_fps);

    ImGui::End();

    ImGui::SFML::Render(*m_renderWindow);
}

void UIManager::Shutdown() {
    if (m_initialized) {
        ImGui::SFML::Shutdown();
        m_initialized = false;
    }
}

void UIManager::SetWindow(sf::RenderWindow& window) {
    if (!window.isOpen()) {
        throw std::invalid_argument("Cannot set closed window");
    }
    m_renderWindow = &window;
}
