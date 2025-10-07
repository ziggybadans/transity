#pragma once

#include "ecs/ISystem.h"
#include <SFML/System/Time.hpp>
#include <entt/entt.hpp>
#include <vector>
#include <SFML/System/Vector2.hpp>

struct PositionComponent;
struct TrainMovementComponent;
struct TrainPhysicsComponent;
struct LineComponent;

class TrainMovementSystem : public ISystem, public IUpdatable {
public:
    explicit TrainMovementSystem(entt::registry& registry);

    void update(sf::Time dt) override;

private:
    sf::Vector2f getPositionAtDistance(const LineComponent& line, float distance);

    entt::registry &_registry;
};