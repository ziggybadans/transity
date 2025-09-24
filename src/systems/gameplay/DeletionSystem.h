#pragma once

#include "ecs/ISystem.h"
#include "event/DeletionEvents.h"
#include <entt/entt.hpp>
#include "event/EventBus.h"
#include "app/GameState.h"

class DeletionSystem : public ISystem {
public:
    DeletionSystem(entt::registry& registry, EventBus& eventBus, GameState& gameState);
    ~DeletionSystem();

private:
    void onDeleteEntity(const DeleteEntityEvent& event);

    entt::registry& _registry;
    EventBus& _eventBus;
    GameState& _gameState;
    entt::scoped_connection _deleteEntityConnection;
};