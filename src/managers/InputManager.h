// InputManager.h
#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

#include "EventManager.h"
#include "../graphics/Camera.h"
#include "../world/WorldMap.h"

class InputManager {
public:
    InputManager(std::shared_ptr<EventManager> eventManager,
        std::shared_ptr<Camera> camera,
        sf::RenderWindow& window,
        std::shared_ptr<WorldMap> worldMap);
    ~InputManager();

    // Configuration setters
    void SetZoomSpeed(float speed);
    void SetPanSpeed(float speed);

    // Call this every frame with the current delta time
    void HandleInput(float deltaTime);

private:
    std::shared_ptr<EventManager> eventManager;
    std::shared_ptr<Camera> camera;
    sf::RenderWindow& window;
    std::shared_ptr<WorldMap> worldMap;

    // Configuration parameters
    float zoomSpeed;
    float panSpeed;

    // Event handlers
    void OnMouseWheelScrolled(const sf::Event& event);
    void OnKeyPressed(const sf::Event& event);
    void OnMouseMoved(const sf::Event& event);
    void OnMouseButtonPressed(const sf::Event& event);


    // Panning state
    bool isPanning;
    sf::Vector2i lastMousePosition;

    // Continuous input state
    sf::Vector2f continuousMovement;

    // Line building state
    Station* startingStation;
};
