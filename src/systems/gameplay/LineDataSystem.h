#pragma once

#include "ecs/ISystem.h"
#include <SFML/System/Time.hpp>
#include <entt/entt.hpp>

class ServiceLocator;

class LineDataSystem : public ISystem, public IUpdatable {
public:
    explicit LineDataSystem(ServiceLocator &serviceLocator);

    void update(sf::Time dt) override;

private:
    entt::registry &_registry;
    void processParallelSegments();
};