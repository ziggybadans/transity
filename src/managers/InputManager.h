#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <functional>
#include <vector>
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
    Select
};

class InputManager {
public:
    InputManager(std::shared_ptr<EventManager> eventMgr, sf::RenderWindow& win);
    ~InputManager();

    /* Input Processing */
    void HandleInput(float deltaTime);
    void RegisterActionCallback(InputAction action, std::function<void()> callback);

    /* Setters */
    void SetZoomSpeed(float speed);
    void SetPanSpeed(float speed);

    /* Getters */
    float GetPanSpeed() const { return m_panSpeed; }
    float GetZoomSpeed() const { return m_zoomSpeed; }

private:
    void EmitAction(InputAction action);

    /* Core Components */
    std::shared_ptr<EventManager> m_eventManager;
    sf::RenderWindow& m_window;

    /* Input Configuration */
    float m_zoomSpeed;
    float m_panSpeed;
    std::vector<std::pair<InputAction, std::function<void()>>> m_actionCallbacks;
};
