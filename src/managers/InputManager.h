#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <functional>
#include <vector>
#include <utility>
#include "EventManager.h"
#include "../graphics/Camera.h"
#include "../world/Map.h"
#include "../Debug.h"
#include <string>
#include "../core/StateManager.h"

enum class InputAction {
    ZoomIn,
    ZoomOut,
    PanLeft,
    PanRight,
    PanUp,
    PanDown,
    Place
};

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

// Concrete commands
class ZoomCommand : public InputCommand {
    std::shared_ptr<Camera> m_camera;
    float m_factor;
public:
    ZoomCommand(std::shared_ptr<Camera> camera, float factor)
        : m_camera(camera), m_factor(factor) {}
    void execute() override { 
        DEBUG_DEBUG("ZoomCommand: Zooming camera by factor ", m_factor);
        m_camera->Zoom(m_factor);
    }
};

class PanCommand : public InputCommand {
    std::shared_ptr<Camera> m_camera;
    sf::Vector2f m_direction;
public:
    PanCommand(std::shared_ptr<Camera> camera, sf::Vector2f direction)
        : m_camera(camera), m_direction(direction) {}
    void execute() override { 
        DEBUG_DEBUG("PanCommand: Panning camera by direction ", m_direction.x, ", ", m_direction.y);
        m_camera->Move(m_direction); 
    }
};

class PlaceCommand : public InputCommand {
    std::shared_ptr<Camera> m_camera;
    sf::RenderWindow& m_window;
    std::shared_ptr<Map> m_map;
public:
    PlaceCommand(std::shared_ptr<Camera> camera, sf::RenderWindow& window, std::shared_ptr<Map> map)
        : m_camera(camera), m_window(window), m_map(map) {}
    void execute() override { 
        sf::Vector2i pixelPos = sf::Mouse::getPosition(m_window);
        sf::Vector2f worldPos = m_window.mapPixelToCoords(pixelPos, m_camera->GetView());
        DEBUG_DEBUG("PlaceCommand: Attempting to place city at world position ", worldPos.x, ", ", worldPos.y);
        m_map->AddCity(worldPos);
    }
};

class InputManager {
public:
    InputManager(std::shared_ptr<EventManager> eventMgr, std::shared_ptr<StateManager> stateMgr,
        sf::RenderWindow& win,
        std::shared_ptr<Camera> camera, std::shared_ptr<Map> map)
        : m_eventManager(eventMgr)
        , m_stateManager(stateMgr)
        , m_window(win)
        , m_camera(camera)
        , m_map(map)
        , m_config{}
    {
        DEBUG_INFO("InputManager: Initializing InputManager");
        m_stateManager->Subscribe("CurrentTool", [this](const std::any& data) {
            if (data.has_value()) {
                auto value = std::any_cast<std::string>(data);
                CheckSubscriptions();
                DEBUG_DEBUG("Now listening for CurrentTool state change on InputManager");
            }
        });
        InitializeSubscriptions();
        InitializeCommands();
    }

    void HandleInput(float deltaTime) {
        // Handle zoom
        float mouseWheel = ImGui::GetIO().MouseWheel;
        if (mouseWheel != 0.0f) {
            DEBUG_VERBOSE("InputManager: Mouse wheel event detected: ", mouseWheel);
            ExecuteCommand(mouseWheel > 0.0f ? InputAction::ZoomOut : InputAction::ZoomIn);
        }

        // Handle movement
        for (const auto& [key, action] : m_keyMappings) {
            if (sf::Keyboard::isKeyPressed(key)) {
                DEBUG_VERBOSE("InputManager: Key pressed: ", key);
                ExecuteCommand(action);
            }
        }
    }

    void SetConfig(const InputConfig& config) {
        DEBUG_INFO("InputManager: Setting new input configuration.");
        DEBUG_DEBUG("InputManager: Zoom Speed: ", config.zoomSpeed, ", Pan Speed: ", config.panSpeed);
        m_config = config;
    }
    const InputConfig& GetConfig() const { return m_config; }

private:
    void InitializeSubscriptions() {
        placeSubscription = m_eventManager->Subscribe(EventType::MouseButtonPressed,
            [this](const EventData& data) {
                if (std::holds_alternative<sf::Event>(data)) {
                    const sf::Event& event = std::get<sf::Event>(data);
                    DEBUG_DEBUG("Subscribing to PlaceSubscription...");

                    if (event.type == sf::Event::MouseButtonPressed) {
                        if (event.mouseButton.button == sf::Mouse::Right) {
                            DEBUG_DEBUG("InputManager: Right mouse button clicked.");
                            ExecuteCommand(InputAction::Place);
                        }
                    }
                }
            }
        );
    }

    void CheckSubscriptions() {
        if (m_stateManager->GetState<std::string>("CurrentTool") == std::string("Place")) {
            placeSubscription = m_eventManager->Subscribe(EventType::MouseButtonPressed,
                [this](const EventData& data) {
                    if (std::holds_alternative<sf::Event>(data)) {
                        const sf::Event& event = std::get<sf::Event>(data);
                        DEBUG_DEBUG("Subscribing to PlaceSubscription...");

                        if (event.type == sf::Event::MouseButtonPressed) {
                            if (event.mouseButton.button == sf::Mouse::Right) {
                                DEBUG_DEBUG("InputManager: Right mouse button clicked.");
                                ExecuteCommand(InputAction::Place);
                            }
                        }
                    }
                }
            );
        }
        else {
            if (m_stateManager->GetState<std::string>("CurrentTool") != std::string("Place")) {
                DEBUG_DEBUG("Unsubscribing...");
                m_eventManager->Unsubscribe(placeSubscription);
                placeSubscription;
            }
        }
    }

    void InitializeCommands() {
        DEBUG_INFO("InputManager: Initializing input commands.");
        // Initialize zoom commands
        m_commands[InputAction::ZoomIn] = std::make_unique<ZoomCommand>(m_camera, m_config.zoomSpeed);
        m_commands[InputAction::ZoomOut] = std::make_unique<ZoomCommand>(m_camera, 1.0f / m_config.zoomSpeed);
        DEBUG_DEBUG("InputManager: Zoom commands initialized.");

        // Initialize pan commands
        m_commands[InputAction::PanLeft] = std::make_unique<PanCommand>(m_camera, sf::Vector2f(-1, 0));
        m_commands[InputAction::PanRight] = std::make_unique<PanCommand>(m_camera, sf::Vector2f(1, 0));
        m_commands[InputAction::PanUp] = std::make_unique<PanCommand>(m_camera, sf::Vector2f(0, -1));
        m_commands[InputAction::PanDown] = std::make_unique<PanCommand>(m_camera, sf::Vector2f(0, 1));
        DEBUG_DEBUG("InputManager: Pan commands initialized.");

        m_commands[InputAction::Place] = std::make_unique<PlaceCommand>(m_camera, m_window, m_map);
        DEBUG_DEBUG("InputManager: Place command initialized.");

        // Initialize key mappings
        m_keyMappings = {
            {sf::Keyboard::Left, InputAction::PanLeft},
            {sf::Keyboard::A, InputAction::PanLeft},
            {sf::Keyboard::Right, InputAction::PanRight},
            {sf::Keyboard::D, InputAction::PanRight},
            {sf::Keyboard::Up, InputAction::PanUp},
            {sf::Keyboard::W, InputAction::PanUp},
            {sf::Keyboard::Down, InputAction::PanDown},
            {sf::Keyboard::S, InputAction::PanDown}
        };
        DEBUG_DEBUG("InputManager: Key mappings initialized.");
    }

    void ExecuteCommand(InputAction action) {
        DEBUG_DEBUG("InputManager: Executing command for action: ", static_cast<int>(action));
        if (auto it = m_commands.find(action); it != m_commands.end()) {
            it->second->execute();
        }
        else {
            DEBUG_WARNING("InputManager: No command found for action: ", static_cast<int>(action));
        }
    }

    std::shared_ptr<EventManager> m_eventManager;
    std::shared_ptr<StateManager> m_stateManager;
    sf::RenderWindow& m_window;
    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<Map> m_map;
    InputConfig m_config;

    std::unordered_map<InputAction, std::unique_ptr<InputCommand>> m_commands;
    std::vector<std::pair<sf::Keyboard::Key, InputAction>> m_keyMappings;

    EventManager::SubscriptionID placeSubscription;
};