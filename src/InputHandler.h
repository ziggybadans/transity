#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <entt/entt.hpp>
#include <optional>
#include <vector>
#include <variant>
#include "Camera.h"
#include "InteractionMode.h"

enum class InputEventType {
    WindowClose,
    CameraZoom,
    CameraPan,
    TryPlaceStation,
    AddStationToLineIntent,
    FinalizeLineIntent,
    None
};

struct InputData {
    float zoomDelta = 0.0f;
    sf::Vector2i mousePixelPosition;
    sf::Vector2f panDirection;
    sf::Vector2f worldPosition;
    entt::entity clickedEntity = entt::null;
};

struct InputCommand {
    InputEventType type = InputEventType::None;
    InputData data;
};

class InputHandler {
public:
    InputHandler();
    void handleGameEvent(const sf::Event& event, InteractionMode currentMode, Camera& camera, sf::RenderWindow& window, entt::registry& registry);
    void update(sf::Time dt);

    const std::vector<InputCommand>& getCommands() const;
    void clearCommands();
    void addCommand(const InputCommand& command);

private:
    std::vector<InputCommand> m_commands;

    float cameraSpeed;
    float zoomFactor;
    float unzoomFactor;
};