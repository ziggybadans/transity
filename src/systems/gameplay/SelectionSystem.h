#pragma once

#include "ecs/ISystem.h"
#include "event/InputEvents.h"
#include "event/EventBus.h"
#include "app/GameState.h"
#include <entt/entt.hpp>

class Pathfinder;

class SelectionSystem : public ISystem {
public:
    explicit SelectionSystem(entt::registry& registry, EventBus& eventBus, GameState& gameState, Pathfinder& pathfinder);
    ~SelectionSystem() override;

    void onMouseButtonPressed(const MouseButtonPressedEvent& event);

private:
    entt::registry& _registry;
    EventBus& _eventBus;
    GameState& _gameState;
    Pathfinder& _pathfinder;
    entt::scoped_connection _mouseButtonConnection;
};