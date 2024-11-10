#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <imgui.h> // Include ImGui header

#include "EventManager.h"
#include "../graphics/Camera.h"
#include "../world/WorldMap.h"

class InputManager {
public:
    // Constructor that initializes the InputManager with references to event manager, camera, window, and world map.
    InputManager(std::shared_ptr<EventManager> eventManager,
        std::shared_ptr<Camera> camera,
        sf::RenderWindow& window,
        std::shared_ptr<WorldMap> worldMap);
    ~InputManager();

    // Set the zoom speed for the camera.
    void SetZoomSpeed(float speed);

    // Set the pan speed for the camera.
    void SetPanSpeed(float speed);

    // Handles user input every frame, using the current delta time for smooth movement.
    void HandleInput(float deltaTime);

private:
    std::shared_ptr<EventManager> eventManager; // Reference to the event manager.
    std::shared_ptr<Camera> camera; // Reference to the camera for manipulating the view.
    sf::RenderWindow& window; // Reference to the SFML RenderWindow.
    std::shared_ptr<WorldMap> worldMap; // Reference to the world map for accessing game data.

    // Configuration parameters for input management.
    float zoomSpeed; // Speed factor for zooming the camera.
    float panSpeed; // Speed factor for panning the camera.

    // Event handlers for different types of input events.
    void OnMouseWheelScrolled(const sf::Event& event); // Handles mouse wheel scrolling.
    void OnKeyPressed(const sf::Event& event); // Handles key press events.
    void OnMouseMoved(const sf::Event& event); // Handles mouse movement.
    void OnMouseButtonPressed(const sf::Event& event); // Handles mouse button presses.
    void OnMouseButtonReleased(const sf::Event& event);

    // State tracking for panning.
    bool isPanning; // Flag to track whether panning is currently active.
    sf::Vector2i lastMousePosition; // Stores the last mouse position for calculating panning movement.

    // State tracking for continuous input.
    sf::Vector2f continuousMovement; // Tracks continuous movement for smooth panning.

    // Line building state.
    Station* startingStation; // Pointer to the starting station for building lines.

    // Line selection state.
    Line* selectedLine; // Pointer to the currently selected line.

    // Node selection and dragging
    bool isDraggingNode = false;
    int selectedNodeIndex = -1;
    Line* editingLine = nullptr; // Pointer to the line currently being edited

    // Station dragging
    bool isDraggingStation = false;
    Station* selectedStation = nullptr; // Pointer to the station currently being moved
};
