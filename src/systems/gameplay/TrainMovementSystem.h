#pragma once

#include "ecs/ISystem.h"
#include <SFML/System/Time.hpp>
#include <entt/entt.hpp> // Add this line

class ServiceLocator;

class TrainMovementSystem : public ISystem, public IUpdatable {
public:
    explicit TrainMovementSystem(ServiceLocator &serviceLocator);

    void update(sf::Time dt) override;

private:
    entt::registry &_registry;
};