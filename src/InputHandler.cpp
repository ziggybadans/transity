#include "InputHandler.h"
#include "Logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <variant>

InputHandler::InputHandler()
    : cameraSpeed(200.0f)
    , zoomFactor(0.9f)
    , unzoomFactor(1.0f / zoomFactor) {
    LOG_INFO("Input", "InputHandler created.");
}

void InputHandler::handleGameEvent(const sf::Event& event, InteractionMode currentMode, Camera& camera, sf::RenderWindow& window) {
    if (event.is<sf::Event::Closed>()) {
        LOG_INFO("Input", "Window close event received.");
        m_commands.push_back({InputEventType::WindowClose, {}});
    } else if (auto* scrollData = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (scrollData->wheel == sf::Mouse::Wheel::Vertical) {
            LOG_DEBUG("Input", "Mouse wheel scrolled: delta %.1f", scrollData->delta);
            InputData data;
            data.mousePixelPosition = sf::Mouse::getPosition(window);
            if (scrollData->delta > 0) {
                data.zoomDelta = zoomFactor;
                LOG_TRACE("Input", "Zoom in command generated.");
            } else if (scrollData->delta < 0) {
                data.zoomDelta = unzoomFactor;
                LOG_TRACE("Input", "Zoom out command generated.");
            }
            if (data.zoomDelta != 0.0f) {
                 m_commands.push_back({InputEventType::CameraZoom, data});
            }
        }
    } else if (auto* pressData = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (pressData->button == sf::Mouse::Button::Right) {
            if (currentMode == InteractionMode::CREATE_STATION) {
                sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePixelPos, camera.getView());
                LOG_DEBUG("Input", "Right mouse button pressed at screen ( %d, %d ), world (%.1f, %.1f). TryPlaceStation command generated.", mousePixelPos.x, mousePixelPos.y, worldPos.x, worldPos.y);
                InputData data;
                data.worldPosition = worldPos;
                m_commands.push_back({InputEventType::TryPlaceStation, data});
            }
        }
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

const std::vector<InputCommand>& InputHandler::getCommands() const {
    return m_commands;
}

void InputHandler::clearCommands() {
    m_commands.clear();
    LOG_TRACE("Input", "Input commands cleared.");
}