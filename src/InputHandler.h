#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <optional>
#include <vector>
#include <variant>
#include "Camera.h"

enum class InputEventType {
    WindowClose,
    CameraZoom,      // data.zoomDelta, data.mousePixelPosition
    CameraPan,       // data.panDirection (scaled by dt)
    TryPlaceStation, // data.worldPosition
    None
};

struct InputData {
    float zoomDelta = 0.0f;
    sf::Vector2i mousePixelPosition;
    sf::Vector2f panDirection;
    sf::Vector2f worldPosition;
};

struct InputCommand {
    InputEventType type = InputEventType::None;
    InputData data;
};

class InputHandler {
public:
    InputHandler(sf::RenderWindow& window, Camera& camera);
    // void processEvents(); // Removed, event loop is now in Game::run()
    void handleEvent(const sf::Event& event); // Now adds to m_commands
    void update(sf::Time dt);
    bool isWindowOpen() const; // This might need to be re-evaluated if window is closed outside event loop

    const std::vector<InputCommand>& getCommands() const;
    void clearCommands();

private:
    sf::RenderWindow& m_window;
    Camera& m_camera; // Kept for mapPixelToCoords and getting view state
    std::vector<InputCommand> m_commands;

    float cameraSpeed; // Will be used to calculate panDirection magnitude
    float zoomFactor;
    float unzoomFactor;
};