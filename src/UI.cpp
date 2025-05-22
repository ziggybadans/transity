#include "UI.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include "Logger.h"
#include "InteractionMode.h"
#include <cstdlib> // For EXIT_FAILURE

UI::UI(sf::RenderWindow& window)
    : m_window(window), m_currentInteractionMode(InteractionMode::SELECT) {
    LOG_INFO("UI", "UI instance created.");
}

UI::~UI() {
    LOG_INFO("UI", "UI instance destroyed.");
}

void UI::init() {
    LOG_INFO("UI", "Initializing ImGui.");
    ImGui::CreateContext();
    if (!ImGui::SFML::Init(m_window)) {
        LOG_FATAL("UI", "Failed to initialize ImGui-SFML");
        exit(EXIT_FAILURE);
    }
    ImGui::StyleColorsDark();
    LOG_INFO("UI", "ImGui initialized successfully.");
}

void UI::processEvent(const sf::Event& event) {
    ImGui::SFML::ProcessEvent(m_window, event);
}

void UI::update(sf::Time deltaTime) {
    ImGui::SFML::Update(m_window, deltaTime);

    ImGui::Begin("Interaction Modes");
    int mode = static_cast<int>(m_currentInteractionMode);

    if (ImGui::RadioButton("None", &mode, static_cast<int>(InteractionMode::SELECT))) {
        m_currentInteractionMode = InteractionMode::SELECT;
        LOG_INFO("UI", "Interaction mode changed to: None");
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Station Placement", &mode, static_cast<int>(InteractionMode::CREATE_STATION))) {
        m_currentInteractionMode = InteractionMode::CREATE_STATION;
        LOG_INFO("UI", "Interaction mode changed to: StationPlacement");
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Line Creation", &mode, static_cast<int>(InteractionMode::CREATE_LINE_START))) {
        m_currentInteractionMode = InteractionMode::CREATE_LINE_START;
        LOG_INFO("UI", "Interaction mode changed to: LineCreation");
    }
    ImGui::End();

    ImGui::Begin("Debug Window");
    ImGui::Text("FPS: %.1f", 1.f / deltaTime.asSeconds());
    ImGui::End();
}

void UI::render() {
    ImGui::SFML::Render(m_window);
}

void UI::cleanup() {
    LOG_INFO("UI", "Shutting down ImGui.");
    ImGui::SFML::Shutdown();
    LOG_INFO("UI", "ImGui shutdown complete.");
}

InteractionMode UI::getInteractionMode() const {
    return m_currentInteractionMode;
}