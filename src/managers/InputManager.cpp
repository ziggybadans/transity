// InputManager.cpp
#include "InputManager.h"
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/View.hpp>
#include <stdexcept>
#include <iostream> // For debug statements if needed

InputManager::InputManager(std::shared_ptr<EventManager> eventMgr,
    std::shared_ptr<Camera> cam,
    sf::RenderWindow& win,
    std::shared_ptr<WorldMap> worldMap)
    : eventManager(eventMgr),
    camera(cam),
    window(win),
    worldMap(worldMap),
    zoomSpeed(1.1f),
    panSpeed(500.0f),
    isPanning(false),
    lastMousePosition(0, 0),
    continuousMovement(0.0f, 0.0f),
    startingStation(nullptr)
{
    // Subscribe to relevant events
    eventManager->Subscribe(EventType::MouseWheelScrolled,
        [this](const sf::Event& event) { this->OnMouseWheelScrolled(event); });

    eventManager->Subscribe(EventType::MouseMoved,
        [this](const sf::Event& event) { this->OnMouseMoved(event); });

    eventManager->Subscribe(EventType::KeyPressed,
        [this](const sf::Event& event) { this->OnKeyPressed(event); });

    eventManager->Subscribe(EventType::MouseButtonPressed,
        [this](const sf::Event& event) { this->OnMouseButtonPressed(event); });
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
    // Handle continuous key presses for panning
    continuousMovement = sf::Vector2f(0.0f, 0.0f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        continuousMovement.x -= panSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        continuousMovement.x += panSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        continuousMovement.y -= panSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
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

void InputManager::OnMouseButtonPressed(const sf::Event& event) {
    if (event.mouseButton.button == sf::Mouse::Right) {
        // Right-click: attempt to build a station in the area
        sf::Vector2i mousePos(event.mouseButton.x, event.mouseButton.y);
        sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, camera->GetView());

        // For simplicity, we'll assume that any click is within an area
        worldMap->AddStation(worldPos);
    }
    else if (event.mouseButton.button == sf::Mouse::Left) {
        // Left-click: either start building a line or add a node
        sf::Vector2i mousePos(event.mouseButton.x, event.mouseButton.y);
        sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, camera->GetView());

        if (!worldMap->IsBuildingLine()) {
            // Check if clicked on a station
            Station* station = worldMap->GetStationAtPosition(worldPos);
            if (station) {
                // Start building a line
                worldMap->StartBuildingLine(station->GetPosition());
                startingStation = station;
            }
        }
        else {
            // Check if clicked on another station
            Station* station = worldMap->GetStationAtPosition(worldPos);
            if (station && station != startingStation) {
                // Add station position to line and finish
                worldMap->AddNodeToCurrentLine(station->GetPosition(), false);
                worldMap->FinishCurrentLine();
                startingStation = nullptr;
            }
            else {
                // Add node to line
                bool isShiftPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
                worldMap->AddNodeToCurrentLine(worldPos, isShiftPressed);
            }
        }
    }
}

void InputManager::OnKeyPressed(const sf::Event& event) {
    if (event.key.code == sf::Keyboard::Enter && worldMap->IsBuildingLine()) {
        // Finish building the line
        worldMap->FinishCurrentLine();
        startingStation = nullptr;
    }
}

void InputManager::OnMouseMoved(const sf::Event& event) {
    if (event.type != sf::Event::MouseMoved)
        return;

    sf::Vector2i currentPos(event.mouseMove.x, event.mouseMove.y);
    sf::Vector2f worldPos = window.mapPixelToCoords(currentPos, camera->GetView());

    if (worldMap->IsBuildingLine()) {
        worldMap->SetCurrentMousePosition(worldPos);
    }

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
