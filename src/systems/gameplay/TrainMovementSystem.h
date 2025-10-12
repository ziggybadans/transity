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
    // Calculates the world position on a line's curve at a specific distance.
    sf::Vector2f getPositionAtDistance(const LineComponent& line, float distance);

    // Updates the train's state (e.g., moving, stopping) and speed based on its situation.
    void updateTrainStateAndSpeed(TrainMovementComponent& movement, TrainPhysicsComponent& physics, const LineComponent& line, sf::Time dt);

    // Updates the train's position along the curve and handles the logic for stopping precisely at a station.
    void updateTrainPositionAndStop(entt::entity trainEntity, sf::Time dt);

    // Finds the distance to the next stop on the line, based on the train's direction.
    std::optional<float> findNextStopDistance(const TrainMovementComponent& movement, const LineComponent& line);

    entt::registry &_registry;
};