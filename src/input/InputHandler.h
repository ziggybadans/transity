#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <entt/entt.hpp>
#include <optional>
#include <vector>
#include <variant>
#include "../core/Camera.h"
#include "../input/InteractionMode.h"
#include "../event/LineEvents.h"

enum class InputEventType {
    WINDOW_CLOSE,
    CAMERA_ZOOM,
    CAMERA_PAN,
    TRY_PLACE_STATION,
    ADD_STATION_TO_LINE_INTENT,
    FINALIZE_LINE_INTENT,
    NONE
};

struct InputData {
    float zoomDelta = 0.0f;
    sf::Vector2i mousePixelPosition;
    sf::Vector2f panDirection;
    sf::Vector2f worldPosition;
    entt::entity clickedEntity = entt::null;
};

struct InputCommand {
    InputEventType type = InputEventType::NONE;
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

    const std::vector<std::variant<AddStationToLineEvent, FinalizeLineEvent>>& getGameEvents() const;
    void clearGameEvents();

private:
    std::vector<InputCommand> _commands;

    float _cameraSpeed;
    float _zoomFactor;
    float _unzoomFactor;

    std::vector<std::variant<AddStationToLineEvent, FinalizeLineEvent>> _gameEvents;
};