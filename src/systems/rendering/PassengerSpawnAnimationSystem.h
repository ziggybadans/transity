#pragma once

#include "ecs/ISystem.h"
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

class EntityFactory;
class Pathfinder;

class PassengerSpawnAnimationSystem : public ISystem, public IUpdatable {
public:
    explicit PassengerSpawnAnimationSystem(entt::registry& registry, EntityFactory& entityFactory, Pathfinder& pathfinder);

    void update(sf::Time dt) override;
    void render(sf::RenderTarget& target);

private:
    entt::registry& _registry;
    EntityFactory& _entityFactory;
    Pathfinder& _pathfinder;
};