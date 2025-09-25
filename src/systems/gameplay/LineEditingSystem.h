#pragma once

#include "ecs/ISystem.h"
#include "event/EventBus.h"
#include "app/GameState.h"
#include "event/InputEvents.h"
#include <entt/entt.hpp>
#include <optional>

class LineEditingSystem : public ISystem {
public:
    LineEditingSystem(entt::registry& registry, EventBus& eventBus, GameState& gameState);
    ~LineEditingSystem();

private:
    void onMouseButtonPressed(const MouseButtonPressedEvent& event);
    void onMouseButtonReleased(const MouseButtonReleasedEvent& event);
    void onMouseMoved(const MouseMovedEvent& event);
    void onKeyPressed(const KeyPressedEvent& event);

    entt::registry& _registry;
    EventBus& _eventBus;
    GameState& _gameState;

    entt::scoped_connection _mouseButtonPressedConnection;
    entt::scoped_connection _mouseButtonReleasedConnection;
    entt::scoped_connection _mouseMovedConnection;
    entt::scoped_connection _keyPressedConnection;

    std::optional<size_t> _draggedPointIndex;
    std::optional<size_t> _selectedPointIndex;
};