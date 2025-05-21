#include "UIManager.h"
#include "imgui.h"
#include "imgui-sfml/imgui-SFML.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "Game.h" // For InteractionMode
#include "Logger.h" // For LOG_INFO


UIManager::UIManager() : m_currentInteractionMode(InteractionMode::None), m_window(nullptr) {
    // Constructor
}

UIManager::~UIManager() {
    // Destructor
    // Shutdown is called explicitly via UIManager::shutdown()
}

void UIManager::init(sf::RenderWindow& window) {
    m_window = &window; // Store the window pointer
    ImGui::SFML::Init(*m_window);
    ImGui::GetIO().IniFilename = nullptr; // Disable imgui.ini
    m_currentInteractionMode = InteractionMode::None; // Default mode
    m_deltaClock.restart(); // Initialize clock
}

void UIManager::processEvent(const sf::Event& event) {
    ImGui::SFML::ProcessEvent(event);
}

void UIManager::update() {
    if (!m_window) return;

    sf::Time dt = m_deltaClock.restart();
    ImGui::SFML::Update(*m_window, dt);

    ImGui::Begin("Interaction Modes");
    int mode = static_cast<int>(m_currentInteractionMode);

    if (ImGui::RadioButton("None", &mode, static_cast<int>(InteractionMode::None))) {
        m_currentInteractionMode = InteractionMode::None;
        LOG_INFO("UIManager", "Interaction mode changed to: %s", "None");
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Station Placement", &mode, static_cast<int>(InteractionMode::StationPlacement))) {
        m_currentInteractionMode = InteractionMode::StationPlacement;
        LOG_INFO("UIManager", "Interaction mode changed to: %s", "StationPlacement");
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Line Creation", &mode, static_cast<int>(InteractionMode::LineCreation))) {
        m_currentInteractionMode = InteractionMode::LineCreation;
        LOG_INFO("UIManager", "Interaction mode changed to: %s", "LineCreation");
    }
    // Add other InteractionMode::Building if it exists and is needed
    // if (ImGui::RadioButton("Building", &mode, static_cast<int>(InteractionMode::Building))) {
    //     m_currentInteractionMode = InteractionMode::Building;
    //     LOG_INFO("UIManager", "Interaction mode changed to: %s", "Building");
    // }

    ImGui::End();

    ImGui::Begin("Debug Window");
    ImGui::Text("FPS: %.1f", 1.f / dt.asSeconds());
    ImGui::End();
}

void UIManager::render() {
    if (!m_window) return; // Should not happen if init is called

    ImGui::SFML::Render(*m_window);
}

InteractionMode UIManager::getCurrentInteractionMode() const {
    return m_currentInteractionMode;
}

void UIManager::shutdown() {
    ImGui::SFML::Shutdown();
}