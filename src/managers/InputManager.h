#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <functional>
#include <vector>
#include <stdexcept>
#include <utility>
#include "EventManager.h"
#include "../graphics/Camera.h"
#include "../world/WorldMap.h"

enum class InputAction {
    ZoomIn,
    ZoomOut,
    PanLeft,
    PanRight,
    PanUp,
    PanDown,
    Select,
    // Add more actions as needed
};

class InputManager {
public:
    // Constructor that initializes the InputManager with references to event manager and window.
    InputManager(std::shared_ptr<EventManager> eventMgr,
                sf::RenderWindow& win);
    ~InputManager();

    // Set the zoom speed for the camera.
    void SetZoomSpeed(float speed);

    // Set the pan speed for the camera.
    void SetPanSpeed(float speed);

    // Handles user input every frame, using the current delta time for smooth movement.
    void HandleInput(float deltaTime);

    // Register callbacks for input actions
    void RegisterActionCallback(InputAction action, std::function<void()> callback);

    // Getters for speeds
    float GetPanSpeed() const { return panSpeed; }
    float GetZoomSpeed() const { return zoomSpeed; }

private:
    std::shared_ptr<EventManager> eventManager; // Reference to the event manager.
    sf::RenderWindow& window;                   // Reference to the SFML RenderWindow.

    // Configuration parameters for input management.
    float zoomSpeed; // Speed factor for zooming the camera.
    float panSpeed;  // Speed factor for panning the camera.

    // Callbacks for input actions
    std::vector<std::pair<InputAction, std::function<void()>>> actionCallbacks;

    // Helper methods
    void EmitAction(InputAction action);
};
