#pragma once

#include "ecs/ISystem.h"
#include "ecs/SystemManager.h"
#include "entt/entt.hpp"
#include "event/InputEvents.h"

class ServiceLocator;

class StationPlacementSystem : public ISystem, public IUpdatable {
public:
    explicit StationPlacementSystem(ServiceLocator &serviceLocator);
    ~StationPlacementSystem();

    void update(sf::Time dt) override;

private:
    void onMouseButtonPressed(const MouseButtonPressedEvent &event);

    entt::registry &_registry;
    EntityFactory &_entityFactory;
    GameState &_gameState;

    entt::connection m_mousePressConnection;
};
