#include "InputManager.h"
#include <SFML/Window/Mouse.hpp>
#include <imgui.h>
#include <iostream>

InputManager::InputManager(std::shared_ptr<EventManager> eventMgr,
                         sf::RenderWindow& win)
    : m_eventManager(eventMgr)
    , m_window(win)
    , m_zoomSpeed(1.1f)
    , m_panSpeed(500.0f)
{
    // Subscribe to relevant events if needed
}

InputManager::~InputManager() 
{
}

void InputManager::HandleInput(float deltaTime) 
{
    // Handle zooming with mouse wheel
    if (ImGui::GetIO().MouseWheel != 0.0f) {
        if (ImGui::GetIO().MouseWheel > 0.0f) {
            EmitAction(InputAction::ZoomIn);
        } else if (ImGui::GetIO().MouseWheel < 0.0f) {
            EmitAction(InputAction::ZoomOut);
        }
    }

    // Handle panning with arrow keys
    sf::Vector2f movement(0.0f, 0.0f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        movement.x -= m_panSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        movement.x += m_panSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        movement.y -= m_panSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        movement.y += m_panSpeed * deltaTime;
    }

    // Emit pan actions based on movement
    if (movement.x != 0.0f || movement.y != 0.0f) {
        if (movement.x < 0.0f) EmitAction(InputAction::PanLeft);
        if (movement.x > 0.0f) EmitAction(InputAction::PanRight);
        if (movement.y < 0.0f) EmitAction(InputAction::PanUp);
        if (movement.y > 0.0f) EmitAction(InputAction::PanDown);
    }
}

void InputManager::RegisterActionCallback(InputAction action, std::function<void()> callback) 
{
    m_actionCallbacks.emplace_back(action, callback);
}

void InputManager::SetZoomSpeed(float speed) 
{
    if (speed <= 0.0f) {
        throw std::invalid_argument("Zoom speed must be positive.");
    }
    m_zoomSpeed = speed;
}

void InputManager::SetPanSpeed(float speed) 
{
    if (speed < 0.0f) {
        throw std::invalid_argument("Pan speed cannot be negative.");
    }
    m_panSpeed = speed;
}

void InputManager::EmitAction(InputAction action) 
{
    for (const auto& pair : m_actionCallbacks) {
        if (pair.first == action) {
            pair.second();
        }
    }
}