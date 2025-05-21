#include "InputHandler.h"
#include "Logger.h"
#include <SFML/Window/Event.hpp> // Explicitly include for sf::Event
// #include "imgui.h" // Removed
// #include "imgui-SFML.h" // Removed
// #include <entt/entt.hpp> // Removed, not used here
#include <iostream> // For potential debug, can be removed if not used
#include <string>   // For potential debug, can be removed if not used
#include <vector>
// #include <variant> // Removed, not using std::get_if anymore

InputHandler::InputHandler(sf::RenderWindow& window, Camera& camera)
    : m_window(window)
    , m_camera(camera)
    , cameraSpeed(200.0f)
    , zoomFactor(0.9f)
    , unzoomFactor(1.0f / zoomFactor) {
    LOG_INFO("Input", "InputHandler created.");
}

// void InputHandler::processEvents() { // Method removed
// }

void InputHandler::handleEvent(const sf::Event& event) {
    // ImGui::SFML::ProcessEvent is now called by UIManager
    switch (event.type) {
        case sf::Event::Closed:
            LOG_INFO("Input", "Window close event received.");
            m_commands.push_back({InputEventType::WindowClose, {}});
            break;
        case sf::Event::MouseWheelScrolled:
            if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::VerticalScroll) {
                LOG_DEBUG("Input", "Mouse wheel scrolled: delta %.1f", event.mouseWheelScroll.delta);
                InputData data;
                data.mousePixelPosition = sf::Mouse::getPosition(m_window);
                if (event.mouseWheelScroll.delta > 0) {
                    data.zoomDelta = zoomFactor;
                    LOG_TRACE("Input", "Zoom in command generated.");
                } else if (event.mouseWheelScroll.delta < 0) {
                    data.zoomDelta = unzoomFactor;
                    LOG_TRACE("Input", "Zoom out command generated.");
                }
                if (data.zoomDelta != 0.0f) {
                    m_commands.push_back({InputEventType::CameraZoom, data});
                }
            }
            break;
        case sf::Event::MouseButtonPressed:
            if (event.mouseButton.button == sf::Mouse::Button::Right) {
                // Check if ImGui wants to capture the mouse
                // This check should ideally be done before UIManager::processEvent if UIManager is to filter,
                // or Game needs to know if ImGui captured it.
                // For now, assume if UIManager processed it, it might have captured it.
                // The task implies InputHandler filters commands by UI mode, but that's now Game's job using UIManager.
                // So, InputHandler just generates raw input commands.
                // If ImGui::GetIO().WantCaptureMouse is true, UIManager should have handled it,
                // and Game might decide not to process this command further based on UI mode.
                // For now, InputHandler will generate the command regardless.
                // if (ImGui::GetIO().WantCaptureMouse) break; // Example of how one might filter

                sf::Vector2i mousePixelPos = {event.mouseButton.x, event.mouseButton.y};
                sf::Vector2f worldPos = m_window.mapPixelToCoords(mousePixelPos, m_camera.getView());
                LOG_DEBUG("Input", "Right mouse button pressed at screen ( %d, %d ), world (%.1f, %.1f). TryPlaceStation command generated.", mousePixelPos.x, mousePixelPos.y, worldPos.x, worldPos.y);
                InputData data;
                data.worldPosition = worldPos;
                m_commands.push_back({InputEventType::TryPlaceStation, data});
            }
            break;
        // Add other event types as needed (e.g., KeyPressed, KeyReleased if not handled by continuous update)
        default:
            break;
    }
}

void InputHandler::update(sf::Time dt) {
    sf::Vector2f panDirection(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        panDirection.y -= 1.0f;
        LOG_TRACE("Input", "Key W pressed.");
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        panDirection.y += 1.0f;
        LOG_TRACE("Input", "Key S pressed.");
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        panDirection.x -= 1.0f;
        LOG_TRACE("Input", "Key A pressed.");
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        panDirection.x += 1.0f;
        LOG_TRACE("Input", "Key D pressed.");
    }

    if (panDirection.x != 0.f || panDirection.y != 0.f) {
        InputData data;
        data.panDirection = panDirection * cameraSpeed * dt.asSeconds();
        m_commands.push_back({InputEventType::CameraPan, data});
        LOG_TRACE("Input", "CameraPan command generated with direction (%.1f, %.1f).", data.panDirection.x, data.panDirection.y);
    }
}

bool InputHandler::isWindowOpen() const {
    return m_window.isOpen();
}

const std::vector<InputCommand>& InputHandler::getCommands() const {
    return m_commands;
}

void InputHandler::clearCommands() {
    m_commands.clear();
    LOG_TRACE("Input", "Input commands cleared.");
}