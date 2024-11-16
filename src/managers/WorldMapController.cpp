#include "WorldMapController.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <imgui.h>

WorldMapController::WorldMapController(std::shared_ptr<WorldMap> map,
                                       std::shared_ptr<InputManager> inputMgr,
                                       std::shared_ptr<Camera> cam,
                                       std::shared_ptr<WindowManager> winMgr)
    : worldMap(map),
      inputManager(inputMgr),
      camera(cam),
      windowManager(winMgr),
      isDraggingLine(false),
      selectedLine(nullptr),
      isDraggingNode(false),
      isDraggingStation(false),
      isPanning(false),
      selectedNodeIndex(-1),
      editingLine(nullptr),
      selectedStation(nullptr),
      startingStation(nullptr)
{}

WorldMapController::~WorldMapController() {}

void WorldMapController::Init() {
    // Register callbacks with InputManager
    inputManager->RegisterActionCallback(InputAction::ZoomIn, std::bind(&WorldMapController::OnZoomIn, this));
    inputManager->RegisterActionCallback(InputAction::ZoomOut, std::bind(&WorldMapController::OnZoomOut, this));
    inputManager->RegisterActionCallback(InputAction::PanLeft, std::bind(&WorldMapController::OnPanLeft, this));
    inputManager->RegisterActionCallback(InputAction::PanRight, std::bind(&WorldMapController::OnPanRight, this));
    inputManager->RegisterActionCallback(InputAction::PanUp, std::bind(&WorldMapController::OnPanUp, this));
    inputManager->RegisterActionCallback(InputAction::PanDown, std::bind(&WorldMapController::OnPanDown, this));
    inputManager->RegisterActionCallback(InputAction::Select, std::bind(&WorldMapController::OnSelect, this));
    inputManager->RegisterActionCallback(InputAction::AddStation, std::bind(&WorldMapController::OnAddStation, this));
}

void WorldMapController::Update(float deltaTime) {
    // Handle any per-frame updates here, such as dragging or panning
    if (isPanning) {
        // Implement panning logic if needed
    }

    // Additional updates can be handled here
}

void WorldMapController::OnZoomIn() {
    float zoomFactor = 1.0f / inputManager->GetZoomSpeed();
    camera->Zoom(zoomFactor);
}

void WorldMapController::OnZoomOut() {
    float zoomFactor = inputManager->GetZoomSpeed();
    camera->Zoom(zoomFactor);
}

void WorldMapController::OnPanLeft() {
    sf::Vector2f movement(-inputManager->GetPanSpeed() * 0.016f, 0.0f); // Assuming 60 FPS
    camera->Move(movement);
}

void WorldMapController::OnPanRight() {
    sf::Vector2f movement(inputManager->GetPanSpeed() * 0.016f, 0.0f);
    camera->Move(movement);
}

void WorldMapController::OnPanUp() {
    sf::Vector2f movement(0.0f, -inputManager->GetPanSpeed() * 0.016f);
    camera->Move(movement);
}

void WorldMapController::OnPanDown() {
    sf::Vector2f movement(0.0f, inputManager->GetPanSpeed() * 0.016f);
    camera->Move(movement);
}

void WorldMapController::OnSelect() {
    // Implement selection logic, e.g., select line or station at mouse position
    SelectObjectAtPosition();
}

void WorldMapController::OnAddStation() {
    // Implement adding a station at mouse position
    AddStationAtPosition();
}

void WorldMapController::SelectObjectAtPosition() {
    // Retrieve mouse position in world coordinates
    sf::Vector2i pixelPos = sf::Mouse::getPosition(windowManager->GetWindow());
    sf::Vector2f worldPos = windowManager->GetWindow().mapPixelToCoords(pixelPos, camera->GetView());

    // Attempt to select a line
    Line* line = worldMap->GetLineAtPosition(worldPos, camera->GetZoomLevel());
    if (line) {
        selectedLine = line;
        std::cout << "Line selected." << std::endl;
        return;
    }

    // Attempt to select a station
    Station* station = worldMap->GetStationAtPosition(worldPos, camera->GetZoomLevel());
    if (station) {
        // Handle station selection
        selectedStation = station;
        std::cout << "Station selected: " << station->GetName() << std::endl;
        return;
    }

    std::cout << "No object selected." << std::endl;
}

void WorldMapController::AddStationAtPosition() {
    // Retrieve mouse position in world coordinates
    sf::Vector2i pixelPos = sf::Mouse::getPosition(windowManager->GetWindow());
    sf::Vector2f worldPos = windowManager->GetWindow().mapPixelToCoords(pixelPos, camera->GetView());

    // Add a new station to the world map
    Station* newStation = worldMap->AddStation(worldPos);
    if (newStation) {
        newStation->SetName("New Station");
        std::cout << "Added new station at position: (" << worldPos.x << ", " << worldPos.y << ")" << std::endl;
    } else {
        std::cerr << "Failed to add new station." << std::endl;
    }
}
