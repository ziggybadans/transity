#pragma once

#include "ecs/ISystem.h"
#include <SFML/System/Time.hpp>
#include <entt/entt.hpp>

class ServiceLocator;
class EntityFactory;

class PassengerSpawnSystem : public ISystem, public IUpdatable {
public:
    explicit PassengerSpawnSystem(ServiceLocator& serviceLocator);

    void update(sf::Time dt) override;

private:
    entt::registry& _registry;
    EntityFactory& _entityFactory;
    sf::Time _spawnTimer;
    sf::Time _spawnInterval;
};