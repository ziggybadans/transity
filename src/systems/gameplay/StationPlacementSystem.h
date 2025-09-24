#pragma once

#include "ecs/ISystem.h"
#include "entt/entt.hpp"
#include "event/InputEvents.h"
#include "app/GameState.h"
#include "event/EventBus.h"

class EntityFactory;

class StationPlacementSystem : public ISystem, public IUpdatable {
public:
    explicit StationPlacementSystem(entt::registry& registry, EntityFactory& entityFactory, GameState& gameState, EventBus& eventBus);
    ~StationPlacementSystem();

    void update(sf::Time dt) override;

private:
    void onMouseButtonPressed(const MouseButtonPressedEvent &event);

    entt::registry &_registry;
    EntityFactory &_entityFactory;
    GameState &_gameState;

    entt::scoped_connection m_mousePressConnection;
};