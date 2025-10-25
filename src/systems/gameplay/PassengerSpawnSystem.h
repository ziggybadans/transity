#pragma once

#include "ecs/ISystem.h"
#include <SFML/System/Time.hpp>
#include <entt/entt.hpp>

class EntityFactory;
class Pathfinder;

class PassengerSpawnSystem : public ISystem, public IUpdatable {
public:
    explicit PassengerSpawnSystem(entt::registry& registry, EntityFactory& entityFactory, Pathfinder& pathfinder);

    void update(sf::Time dt) override;
    sf::Time getSpawnTimer() const;
    sf::Time getSpawnInterval() const;
    void setSpawnTimer(sf::Time timer);
    void setSpawnInterval(sf::Time interval);

private:
    entt::registry& _registry;
    EntityFactory& _entityFactory;
    Pathfinder& _pathfinder;
    sf::Time _spawnTimer;
    sf::Time _spawnInterval;
};
