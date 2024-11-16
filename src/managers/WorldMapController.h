#pragma once

#include <memory>
#include "../world/WorldMap.h"
#include "InputManager.h"
#include "../graphics/Camera.h"
#include "WindowManager.h"

class WorldMapController {
public:
    WorldMapController(std::shared_ptr<WorldMap> map,
                       std::shared_ptr<InputManager> inputMgr,
                       std::shared_ptr<Camera> cam,
                       std::shared_ptr<WindowManager> winMgr);
    ~WorldMapController();

    // Initialize the controller by registering callbacks
    void Init();

    // Handle events that require per-frame updates
    void Update(float deltaTime);

private:
    std::shared_ptr<WorldMap> worldMap;
    std::shared_ptr<InputManager> inputManager;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<WindowManager> windowManager;

    // Internal state for dragging and panning
    bool isDraggingLine;
    bool isDraggingNode;
    bool isDraggingStation;
    bool isPanning;
    Line* selectedLine;
    Station* selectedStation;
    int selectedNodeIndex;
    Line* editingLine;
    sf::Vector2i lastMousePosition;
    sf::Vector2f continuousMovement;
    Station* startingStation;

    // Callback methods
    void OnZoomIn();
    void OnZoomOut();
    void OnPanLeft();
    void OnPanRight();
    void OnPanUp();
    void OnPanDown();
    void OnSelect();
    void OnAddStation();

    // Helper methods
    void SelectObjectAtPosition();
    void AddStationAtPosition();
    void HandleMouseMovement(const sf::Event& event);
    void HandleMouseButtonPressed(const sf::Event& event);
    void HandleMouseButtonReleased(const sf::Event& event);
    void HandleKeyPressed(const sf::Event& event);
};
