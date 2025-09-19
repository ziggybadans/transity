#pragma once

#include "ecs/ISystem.h"
#include <SFML/System/Time.hpp>
#include <entt/entt.hpp>

class ServiceLocator;

class PassengerMovementSystem : public ISystem, public IUpdatable {
public:
    explicit PassengerMovementSystem(ServiceLocator& serviceLocator);

    void update(sf::Time dt) override;

private:
    entt::registry& _registry;
};