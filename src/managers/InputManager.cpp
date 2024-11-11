#include "InputManager.h"
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/View.hpp>
#include <stdexcept>
#include <iostream>
#include <imgui.h> // Include ImGui header

/**
<summary>
InputManager is responsible for handling all user input for the application, including keyboard and mouse events.
It interacts with the EventManager to subscribe to relevant input events and manages the camera, the world map,
and game elements like stations and lines.
This class abstracts the complexity of processing input and applying the resulting actions to the game.
</summary>
*/
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
    startingStation(nullptr),
    selectedLine(nullptr) // Initialize selectedLine
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

    eventManager->Subscribe(EventType::MouseButtonReleased,
        [this](const sf::Event& event) { this->OnMouseButtonReleased(event); });
}

/**
<summary>
Destructor for InputManager. Cleans up any resources allocated by InputManager.
</summary>
*/
InputManager::~InputManager() {}

/**
<summary>
Sets the zoom speed for the camera when the mouse wheel is scrolled.
</summary>
<param name="speed">The new zoom speed value. Must be positive.</param>
<exception cref="std::invalid_argument">Thrown if the zoom speed is not positive.</exception>
*/
void InputManager::SetZoomSpeed(float speed) {
    if (speed <= 0.0f) {
        throw std::invalid_argument("Zoom speed must be positive.");
    }
    zoomSpeed = speed;
}

/**
<summary>
Sets the pan speed for moving the camera using keyboard inputs.
</summary>
<param name="speed">The new pan speed value. Must be non-negative.</param>
<exception cref="std::invalid_argument">Thrown if the pan speed is negative.</exception>
*/
void InputManager::SetPanSpeed(float speed) {
    if (speed < 0.0f) {
        throw std::invalid_argument("Pan speed cannot be negative.");
    }
    panSpeed = speed;
}

/**
<summary>
Handles continuous input for panning the camera, such as holding down WASD keys.
This method is called every frame to apply movement to the camera based on user input.
</summary>
<param name="deltaTime">Time elapsed since the last frame, used to calculate movement proportionally.</param>
*/
void InputManager::HandleInput(float deltaTime) {
    // Check if ImGui wants to capture keyboard input
    if (ImGui::GetIO().WantCaptureKeyboard) {
        // ImGui is handling the keyboard input, so we don't process it in the game
        return;
    }

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

/**
<summary>
Handles mouse wheel scrolling to zoom in or out of the camera view.
The zoom is applied in a way that keeps the mouse position stable.
</summary>
<param name="event">The SFML event containing mouse wheel scroll data.</param>
*/
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

/**
<summary>
Handles mouse button press events to interact with game elements such as stations and lines.
Right-click is used to add stations, while left-click selects lines or starts building a new line.
</summary>
<param name="event">The SFML event containing mouse button press data.</param>
*/
void InputManager::OnMouseButtonPressed(const sf::Event& event) {
    if (ImGui::GetIO().WantCaptureMouse) {
        // ImGui is handling the mouse, do not process for game
        return;
    }

    sf::Vector2i mousePos(event.mouseButton.x, event.mouseButton.y);
    sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, camera->GetView());
    float currentZoom = camera->GetZoomLevel();

    bool isShiftHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);

    if (event.mouseButton.button == sf::Mouse::Right) {
        if (isShiftHeld) {
            // Shift + Right-click to finish building the line
            if (worldMap->IsBuildingLine()) {
                worldMap->FinishCurrentLine();
                startingStation = nullptr;
            }
        }
        else {
            // Right-click to add a new station
            worldMap->AddStation(worldPos);
        }
    }
    else if (event.mouseButton.button == sf::Mouse::Left) {
        if (isShiftHeld) {
            // Shift + Left-click to start or continue building a line
            if (!worldMap->IsBuildingLine()) {
                // Check if user clicked on a node or station on an existing line
                Line* line = worldMap->GetLineAtPosition(worldPos, currentZoom);
                if (line) {
                    int nodeIndex = line->GetNodeIndexAtPosition(worldPos, currentZoom);
                    if (nodeIndex != -1) {
                        const std::vector<LineNode>& nodes = line->GetNodes();
                        if (nodeIndex == 0 || nodeIndex == static_cast<int>(nodes.size()) - 1) {
                            // Clicked on the start or end node - extend the main line
                            worldMap->StartExtendingLine(line, nodeIndex);
                        }
                        else {
                            // Clicked on a middle node - create a branch line
                            const LineNode& startingNode = nodes[nodeIndex];
                            worldMap->StartBuildingBranch(line, startingNode);
                        }
                    }
                }
                else {
                    // Start building line from a station
                    Station* station = worldMap->GetStationAtPosition(worldPos, currentZoom);
                    if (station) {
                        worldMap->StartBuildingLine(station);
                        startingStation = station;
                    }
                }
            }
            else {
                // Add station or a node to current line
                Station* station = worldMap->GetStationAtPosition(worldPos, currentZoom);
                if (station && station != startingStation) {
                    worldMap->AddStationToCurrentLine(station);
                }
                else {
                    worldMap->AddNodeToCurrentLine(worldPos);
                }
            }
        }
        else {
            if (selectedLine && selectedLine->IsEditing()) {
                // In edit mode, check if a node is clicked
                int nodeIndex = selectedLine->GetNodeIndexAtPosition(worldPos, currentZoom);
                if (selectedLine && selectedLine->IsEditing()) {
                    // In edit mode, check if a node is clicked
                    int nodeIndex = selectedLine->GetNodeIndexAtPosition(worldPos, currentZoom);
                    if (nodeIndex != -1) {
                        const auto& nodes = selectedLine->GetNodes();
                        if (!nodes[nodeIndex].IsStation()) {
                            // Node is selected and is not a station
                            isDraggingNode = true;
                            selectedNodeIndex = nodeIndex;
                            editingLine = selectedLine;
                        }
                        else {
                            // Node is a station
                            // Start dragging the station
                            Station* station = nodes[nodeIndex].station;
                            if (station) {
                                worldMap->SetSelectedStation(station);
                                isDraggingStation = true;
                                selectedStation = station;
                                editingLine = selectedLine;
                            }
                        }
                    }
                    else {
                        // Handle other clicks if necessary
                    }
                }
                else {
                    // Check if a station is clicked
                    Station* station = worldMap->GetStationAtPosition(worldPos, currentZoom);
                    if (station) {
                        // Select the station
                        worldMap->SetSelectedStation(station);
                        // Deselect the line
                        if (selectedLine) {
                            worldMap->SetSelectedLine(nullptr);
                            selectedLine = nullptr;
                        }
                    }
                }
            }
            else {
                // Left-click without Shift: select station or line
                Station* station = worldMap->GetStationAtPosition(worldPos, currentZoom);
                if (station) {
                    // Select the station
                    worldMap->SetSelectedStation(station);
                    // Deselect the line
                    if (selectedLine) {
                        worldMap->SetSelectedLine(nullptr);
                        selectedLine = nullptr;
                    }
                }
                else {
                    // Check if user clicked on a line to select it
                    Line* line = worldMap->GetLineAtPosition(worldPos, currentZoom);
                    if (line) {
                        worldMap->SetSelectedLine(line);
                        selectedLine = line;
                        // Deselect the station
                        worldMap->SetSelectedStation(nullptr);
                    }
                    else {
                        // Clicked outside any line or station, so deselect both
                        if (selectedLine) {
                            worldMap->SetSelectedLine(nullptr);
                            selectedLine = nullptr;
                        }
                        worldMap->SetSelectedStation(nullptr);
                    }
                }
            }
        }
    }
}

/**
<summary>
Handles key press events. Pressing 'E' toggles edit mode for the selected line.
</summary>
<param name="event">The SFML event containing key press data.</param>
*/
void InputManager::OnKeyPressed(const sf::Event& event) {
    // Check if ImGui wants to capture keyboard input
    if (ImGui::GetIO().WantCaptureKeyboard) {
        // ImGui is handling the keyboard input, so we don't process it in the game
        return;
    }

    // Handle the 'E' key to toggle edit mode for the selected line.
    if (event.key.code == sf::Keyboard::E) {
        Line* selectedLine = worldMap->GetSelectedLine();
        if (selectedLine) {
            bool isEditing = selectedLine->IsEditing();
            selectedLine->SetEditing(!isEditing);
        }
    }
}

/**
<summary>
Handles mouse movement events, used for tasks like updating the position of the currently building line.
Also handles panning the view when the middle mouse button is pressed.
</summary>
<param name="event">The SFML event containing mouse movement data.</param>
*/
void InputManager::OnMouseMoved(const sf::Event& event) {
    if (event.type != sf::Event::MouseMoved)
        return;

    sf::Vector2i currentPos(event.mouseMove.x, event.mouseMove.y);
    sf::Vector2f worldPos = window.mapPixelToCoords(currentPos, camera->GetView());

    if (worldMap->IsBuildingLine()) {
        worldMap->SetCurrentMousePosition(worldPos);
    }

    // Detect if Shift key is held during mouse movement for preview
    bool isShiftHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);

    if (worldMap->IsBuildingLine()) {
        worldMap->SetNextSegmentCurved(isShiftHeld);
    }

    if (isDraggingNode && editingLine) {
        // Update the position of the selected node
        editingLine->SetNodePosition(selectedNodeIndex, worldPos);
    }
    if (isDraggingStation && selectedStation) {
        // Update the position of the selected station
        selectedStation->SetPosition(worldPos);

        // Regenerate spline points of the line being edited
        if (editingLine) {
            editingLine->GenerateSplinePoints();
        }
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

/**
<summary>
Handles mouse button release events, used to stop dragging nodes or stations.
</summary>
<param name="event">The SFML event containing mouse button release data.</param>
*/
void InputManager::OnMouseButtonReleased(const sf::Event& event) {
    if (event.mouseButton.button == sf::Mouse::Left) {
        if (isDraggingNode) {
            isDraggingNode = false;
            selectedNodeIndex = -1;
            editingLine = nullptr;
        }
        if (isDraggingStation) {
            isDraggingStation = false;
            selectedStation = nullptr;
            editingLine = nullptr; // Reset editingLine here
        }
    }
}