#pragma once

#include "ecs/ISystem.h"
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

class ServiceLocator;

class PassengerSpawnAnimationSystem : public ISystem, public IUpdatable {
public:
    explicit PassengerSpawnAnimationSystem(ServiceLocator& serviceLocator);

    void update(sf::Time dt) override;
    void render(sf::RenderTarget& target);

private:
    entt::registry& _registry;
    class EntityFactory& _entityFactory;
    class Pathfinder& _pathfinder;
};