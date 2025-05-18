#include "InputHandler.h"
#include "Logger.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include <entt/entt.hpp> // May not be needed here anymore
#include <iostream>
#include <string>
#include <vector> // Added for std::vector

InputHandler::InputHandler(sf::RenderWindow& window, Camera& camera)
    : m_window(window)
    , m_camera(camera)
    , cameraSpeed(200.0f)
    , zoomFactor(0.9f)
    , unzoomFactor(1.0f / zoomFactor) {
    LOG_INFO("Input", "InputHandler created.");
}

void InputHandler::processEvents() {
    LOG_TRACE("Input", "Processing events.");
    sf::Event event;
    while (m_window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);
        if (event.type == sf::Event::Closed) {
            LOG_INFO("Input", "Window close event received.");
            m_commands.push_back({InputEventType::WindowClose, {}});
        }
        handleEvent(event);
    }
}

void InputHandler::handleEvent(const sf::Event& event) {
    // Camera view is needed for mapPixelToCoords, but we don't modify it directly here.
    // const sf::View& view = m_camera.getView(); // Assuming Camera has a const getView()

    if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
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
    } else if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Right) {
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(m_window);
            // We need the current view from the camera to map pixel to world coordinates
            // This is a read-only operation on the camera's state.
            sf::Vector2f worldPos = m_window.mapPixelToCoords(mousePixelPos, m_camera.getView());
            LOG_DEBUG("Input", "Right mouse button pressed at screen ( %d, %d ), world (%.1f, %.1f). TryPlaceStation command generated.", mousePixelPos.x, mousePixelPos.y, worldPos.x, worldPos.y);
            InputData data;
            data.worldPosition = worldPos;
            m_commands.push_back({InputEventType::TryPlaceStation, data});
        }
    }
}

void InputHandler::update(sf::Time dt) {
    sf::Vector2f panDirection(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        panDirection.y -= 1.0f; // Direction, magnitude applied by consumer
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
        data.panDirection = panDirection * cameraSpeed * dt.asSeconds(); // Pre-scale by speed and dt
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