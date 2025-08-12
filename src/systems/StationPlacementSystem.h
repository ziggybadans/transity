

#pragma once

#include "../core/ISystem.h"
#include "../core/SystemManager.h"
#include "../event/InputEvents.h"
#include "entt/entt.hpp"

class ServiceLocator;

class StationPlacementSystem : public ISystem {
public:
    explicit StationPlacementSystem(ServiceLocator &serviceLocator);
    ~StationPlacementSystem();

    void update(sf::Time dt) override;

private:
    void onMouseButtonPressed(const MouseButtonPressedEvent &event);

    entt::registry *_registry;
    class EntityFactory *_entityFactory;
    class GameState *_gameState;

    entt::connection m_mousePressConnection;
};
