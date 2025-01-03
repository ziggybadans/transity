#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <functional>
#include <vector>
#include <utility>
#include <unordered_map>
#include <string>
#include <any>
#include "EventManager.h"
#include "../graphics/Camera.h"
#include "../world/Map.h"
#include "../Debug.h"
#include "../core/StateManager.h"

// Enumeration for different input actions
enum class InputAction {
    ZoomIn,
    ZoomOut,
    PanLeft,
    PanRight,
    PanUp,
    PanDown,
    Place,
    Draw,
    Select,
    Move
};

// Configuration structure for input settings
struct InputConfig {
    float zoomSpeed = 1.1f;
    float panSpeed = 500.0f;
    // Add other config parameters here
};

// Command interface
class InputCommand {
public:
    virtual ~InputCommand() = default;
    virtual void execute() = 0;
};

// Concrete command for zooming
class ZoomCommand : public InputCommand {
    std::shared_ptr<Camera> m_camera;
    float m_factor;
public:
    ZoomCommand(std::shared_ptr<Camera> camera, float factor);
    void execute() override;
};

// Concrete command for panning
class PanCommand : public InputCommand {
    std::shared_ptr<Camera> m_camera;
    sf::Vector2f m_direction;
public:
    PanCommand(std::shared_ptr<Camera> camera, sf::Vector2f direction);
    void execute() override;
};

// Concrete command for placing objects
class PlaceCommand : public InputCommand {
    std::shared_ptr<Camera> m_camera;
    sf::RenderWindow& m_window;
    std::shared_ptr<Map> m_map;
public:
    PlaceCommand(std::shared_ptr<Camera> camera, sf::RenderWindow& window, std::shared_ptr<Map> map);
    void execute() override;
};

// Concrete command for drawing
class DrawCommand : public InputCommand {
    std::shared_ptr<Camera> m_camera;
    sf::RenderWindow& m_window;
    std::shared_ptr<Map> m_map;
public:
    DrawCommand(std::shared_ptr<Camera> camera, sf::RenderWindow& window, std::shared_ptr<Map> map);
    void execute() override;
};

class SelectCommand : public InputCommand {
    std::shared_ptr<Camera> m_camera;
    sf::RenderWindow& m_window;
    std::shared_ptr<Map> m_map;
public:
    SelectCommand(std::shared_ptr<Camera> camera, sf::RenderWindow& window, std::shared_ptr<Map> map);
    void execute() override;
};

class MoveCommand : public InputCommand {
    std::shared_ptr<Camera> m_camera;
    sf::RenderWindow& m_window;
    std::shared_ptr<Map> m_map;
public:
    MoveCommand(std::shared_ptr<Camera> camera, sf::RenderWindow& window, std::shared_ptr<Map> map);
    void execute() override;
};

// InputManager class declaration
class InputManager {
public:
    InputManager(std::shared_ptr<EventManager> eventMgr,
        std::shared_ptr<StateManager> stateMgr,
        sf::RenderWindow& win,
        std::shared_ptr<Camera> camera,
        std::shared_ptr<Map> map);

    void HandleInput(float deltaTime);
    void SetConfig(const InputConfig& config);
    const InputConfig& GetConfig() const;

private:
    void InitializeSubscriptions();
    void CheckSubscriptions();
    void InitializeCommands();
    void ExecuteCommand(InputAction action);

    std::shared_ptr<EventManager> m_eventManager;
    std::shared_ptr<StateManager> m_stateManager;
    sf::RenderWindow& m_window;
    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<Map> m_map;
    InputConfig m_config;

    std::unordered_map<InputAction, std::unique_ptr<InputCommand>> m_commands;
    std::vector<std::pair<sf::Keyboard::Key, InputAction>> m_keyMappings;

    EventManager::SubscriptionID placeSubscription;
    EventManager::SubscriptionID drawSubscription;

    bool m_isDragging;
    sf::Vector2f m_lastMousePos;
    float m_dragThreshold = 5.0f;
};
