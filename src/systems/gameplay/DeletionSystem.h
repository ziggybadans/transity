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
    void onDeleteAllEntities(const DeleteAllEntitiesEvent& event);
    void handleLineDeletion(entt::entity lineEntity);
    void deleteTrainsOnLine(entt::entity lineEntity);
    void removeLineFromCities(entt::entity lineEntity);
    void repathPassengersAfterLineDeletion(entt::entity lineEntity);

    entt::registry& _registry;
    EventBus& _eventBus;
    GameState& _gameState;
    entt::scoped_connection _deleteEntityConnection;
    entt::scoped_connection _deleteAllEntitiesConnection;
};