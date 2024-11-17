#include "UIManager.h"
#include <string>
#include <iostream>

UIManager::UIManager()
    : m_initialized(false)
    , m_renderWindow(nullptr)
    , m_timeScalePtr(nullptr)
    , m_fps(0.0f)
{
}

UIManager::~UIManager() {
    Shutdown();
}

bool UIManager::Init() {
    if (!m_renderWindow) {
        std::cerr << "No render window set for UIManager" << std::endl;
        return false;
    }

    try {
        if (!ImGui::SFML::Init(*m_renderWindow)) {
            std::cerr << "Failed to initialize ImGui SFML." << std::endl;
            return false;
        }
        m_initialized = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception during ImGui initialization: " << e.what() << std::endl;
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
        std::cerr << "Error updating UI: " << e.what() << std::endl;
        Shutdown(); // Safely shutdown on error
    }
}

void UIManager::Render() {
    if (!m_initialized || !m_renderWindow) {
        return;
    }

    try {
        ImGui::Begin("Performance");
        ImGui::Text("FPS: %.1f", m_fps);
        ImGui::End();

        ImGui::SFML::Render(*m_renderWindow);
    } catch (const std::exception& e) {
        std::cerr << "Error rendering UI: " << e.what() << std::endl;
        Shutdown(); // Safely shutdown on error
    }
}

void UIManager::Shutdown() {
    if (m_initialized) {
        try {
            ImGui::SFML::Shutdown();
        } catch (const std::exception& e) {
            std::cerr << "Error during ImGui shutdown: " << e.what() << std::endl;
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
