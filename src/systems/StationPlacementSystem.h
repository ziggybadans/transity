#pragma once

#include "entt/entt.hpp"
#include "../core/GameState.h"
#include "../event/EventBus.h"
#include "../event/InputEvents.h"

class EntityFactory;

class StationPlacementSystem {
public:
    StationPlacementSystem(entt::registry& registry, EntityFactory& entityFactory, GameState& gameState, EventBus& eventBus);
    ~StationPlacementSystem();

    // Update is no longer needed

private:
    void onTryPlaceStation(const TryPlaceStationEvent& event);

    entt::registry& _registry;
    EntityFactory& _entityFactory;
    GameState& _gameState;
    entt::connection m_placeStationConnection;
};
