// InputManager.cpp
#include "InputManager.h"
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/View.hpp>
#include <stdexcept>

InputManager::InputManager(std::shared_ptr<EventManager> eventMgr,
    std::shared_ptr<Camera> cam,
    sf::RenderWindow& win)
    : eventManager(eventMgr),
    camera(cam),
    window(win),
    zoomSpeed(1.1f),
    panSpeed(500.0f),
    minZoomLevel(0.5f),
    maxZoomLevel(3.0f),
    isPanning(false),
    lastMousePosition(0, 0),
    continuousMovement(0.0f, 0.0f)
{
    // Subscribe to relevant events
    eventManager->Subscribe(EventType::MouseWheelScrolled,
        [this](const sf::Event& event) { this->OnMouseWheelScrolled(event); });

    eventManager->Subscribe(EventType::MouseMoved,
        [this](const sf::Event& event) { this->OnMouseMoved(event); });

    eventManager->Subscribe(EventType::KeyPressed,
        [this](const sf::Event& event) { this->OnKeyPressed(event); });
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

void InputManager::SetMinZoom(float minZoom) {
    if (minZoom <= 0.0f) {
        throw std::invalid_argument("Minimum zoom level must be positive.");
    }
    minZoomLevel = minZoom;
}

void InputManager::SetMaxZoom(float maxZoom) {
    if (maxZoom <= 0.0f) {
        throw std::invalid_argument("Maximum zoom level must be positive.");
    }
    maxZoomLevel = maxZoom;
}

void InputManager::HandleInput(float deltaTime) {
    // Handle continuous key presses for panning
    continuousMovement = sf::Vector2f(0.0f, 0.0f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        continuousMovement.x -= panSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        continuousMovement.x += panSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        continuousMovement.y -= panSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        continuousMovement.y += panSpeed * deltaTime;
    }

    if (continuousMovement != sf::Vector2f(0.0f, 0.0f)) {
        // Adjust movement based on zoom level to ensure consistent panning speed
        float zoomFactor = camera->GetZoomLevel();
        sf::Vector2f movement = continuousMovement * zoomFactor;

        sf::Vector2f newPosition = camera->GetPosition() + movement;
        camera->SetPosition(newPosition);
    }
}

void InputManager::OnMouseWheelScrolled(const sf::Event& event) {
    if (event.type != sf::Event::MouseWheelScrolled)
        return;

    float delta = event.mouseWheelScroll.delta;
    float zoomFactor = (delta > 0) ? (1.0f / zoomSpeed) : zoomSpeed;

    // Get the current mouse position in window coordinates
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);

    // Convert to world coordinates before zoom
    sf::Vector2f beforeZoom = window.mapPixelToCoords(pixelPos, camera->GetView());

    // Apply zoom
    camera->Zoom(zoomFactor);

    // Convert to world coordinates after zoom
    sf::Vector2f afterZoom = window.mapPixelToCoords(pixelPos, camera->GetView());

    // Calculate the offset to keep the mouse position stable
    sf::Vector2f offset = beforeZoom - afterZoom;

    // Adjust the camera position
    sf::Vector2f newCameraPos = camera->GetPosition() + offset;
    camera->SetPosition(newCameraPos);
}

void InputManager::OnKeyPressed(const sf::Event& event) {
    // This method can be used for discrete key presses if needed
}

void InputManager::OnMouseMoved(const sf::Event& event) {
    if (event.type != sf::Event::MouseMoved)
        return;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) { // Use middle mouse for panning
        if (!isPanning) {
            isPanning = true;
            lastMousePosition = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
        }
        else {
            sf::Vector2i currentPos(event.mouseMove.x, event.mouseMove.y);
            sf::Vector2i delta = lastMousePosition - currentPos;
            lastMousePosition = currentPos;

            // Adjust camera position based on mouse movement
            float zoomFactor = camera->GetZoomLevel();
            sf::Vector2f movement(static_cast<float>(delta.x), static_cast<float>(delta.y));
            movement *= zoomFactor;

            sf::Vector2f newPosition = camera->GetPosition() + movement;
            camera->SetPosition(newPosition);
        }
    }
    else {
        isPanning = false;
    }
}