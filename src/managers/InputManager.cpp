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
    // Zooming with mouse wheel
    if (ImGui::GetIO().MouseWheel != 0.0f) {
        if (ImGui::GetIO().MouseWheel > 0.0f) {
            EmitAction(InputAction::ZoomIn);
        }
        else if (ImGui::GetIO().MouseWheel < 0.0f) {
            EmitAction(InputAction::ZoomOut);
        }
    }

    // Panning with arrow keys
    sf::Vector2f movement(0.0f, 0.0f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        movement.x -= panSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        movement.x += panSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        movement.y -= panSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        movement.y += panSpeed * deltaTime;
    }

    if (movement.x != 0.0f || movement.y != 0.0f) {
        // Assume you have access to the Camera instance here
        // For example, through a shared pointer or other means
        // This might require additional design changes
        // Here, we'll emit actions for panning
        if (movement.x < 0.0f) EmitAction(InputAction::PanLeft);
        if (movement.x > 0.0f) EmitAction(InputAction::PanRight);
        if (movement.y < 0.0f) EmitAction(InputAction::PanUp);
        if (movement.y > 0.0f) EmitAction(InputAction::PanDown);
    }

    // Handle other continuous inputs as needed
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