#pragma once

#include "ecs/ISystem.h"
#include <SFML/System/Time.hpp>
#include <entt/entt.hpp>

class ScoreSystem : public ISystem, public IUpdatable {
public:
    explicit ScoreSystem(entt::registry& registry);

    void update(sf::Time dt) override;

private:
    entt::registry& _registry;
    entt::entity _scoreEntity;
};