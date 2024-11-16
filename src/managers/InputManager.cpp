#include "InputManager.h"
#include <SFML/Window/Mouse.hpp>
#include <imgui.h>
#include <iostream>

InputManager::InputManager(std::shared_ptr<EventManager> eventMgr,
                           sf::RenderWindow& win)
    : eventManager(eventMgr),
      window(win),
      zoomSpeed(1.1f),
      panSpeed(500.0f)
{
    // Subscribe to relevant events if needed
}

InputManager::~InputManager() {}

void InputManager::SetZoomSpeed(float speed) {
    if (speed <= 0.0f) {
        throw std::invalid_argument("Zoom speed must be positive.");
    }
    zoomSpeed = speed;
}

void InputManager::SetPanSpeed(float speed) {
    if (speed < 0.0f) {
        throw std::invalid_argument("Pan speed cannot be negative.");
    }
    panSpeed = speed;
}

void InputManager::HandleInput(float deltaTime) {
    // Check if ImGui wants to capture keyboard or mouse input
    if (ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse) {
        // ImGui is handling the input, so we don't process it in the game
        return;
    }

    // Handle continuous key presses for panning
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        EmitAction(InputAction::PanLeft);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        EmitAction(InputAction::PanRight);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        EmitAction(InputAction::PanUp);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        EmitAction(InputAction::PanDown);
    }

    // Handle mouse input
    sf::Event event;
    while (window.pollEvent(event)) {
        // Dispatch events to EventManager if necessary
        eventManager->Dispatch(event); // Existing event dispatching

        if (event.type == sf::Event::MouseWheelScrolled) {
            if (event.mouseWheelScroll.delta > 0) {
                EmitAction(InputAction::ZoomIn);
            }
            else {
                EmitAction(InputAction::ZoomOut);
            }
        }

        // Handle mouse button presses
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                EmitAction(InputAction::Select);
            }
            if (event.mouseButton.button == sf::Mouse::Right) {
                EmitAction(InputAction::AddStation);
            }
        }

        // Handle other relevant events as needed
    }
}

void InputManager::RegisterActionCallback(InputAction action, std::function<void()> callback) {
    actionCallbacks.emplace_back(action, callback);
}

void InputManager::EmitAction(InputAction action) {
    for (const auto& pair : actionCallbacks) {
        if (pair.first == action) {
            pair.second();
        }
    }
}