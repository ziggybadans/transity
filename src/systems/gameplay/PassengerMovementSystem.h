// src/systems/gameplay/PassengerMovementSystem.h

#pragma once

#include "ecs/ISystem.h"
#include <SFML/System/Time.hpp>
#include <entt/entt.hpp>

struct ServiceLocator;
struct LineComponent;
struct TrainMovementComponent;
struct TrainCapacityComponent;

class PassengerMovementSystem : public ISystem, public IUpdatable {
public:
    explicit PassengerMovementSystem(ServiceLocator& serviceLocator);

    void update(sf::Time dt) override;

private:
    void alightPassengers(entt::entity trainEntity, const TrainMovementComponent& movement, TrainCapacityComponent& capacity);
    void boardPassengers(entt::entity trainEntity, const TrainMovementComponent& movement, TrainCapacityComponent& capacity);

    bool isTrainGoingToNextNode(const TrainMovementComponent& movement, const LineComponent& line, entt::entity currentStopEntity, entt::entity nextNodeInPath);

    entt::registry& _registry;
};