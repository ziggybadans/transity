// src/systems/gameplay/TrainMovementSystem.h

#pragma once

#include "ecs/ISystem.h"
#include <SFML/System/Time.hpp>
#include <entt/entt.hpp>

struct ServiceLocator;
struct PositionComponent;
struct TrainMovementComponent;
struct TrainPhysicsComponent;
struct StationApproachComponent;

class TrainMovementSystem : public ISystem, public IUpdatable {
public:
    explicit TrainMovementSystem(ServiceLocator &serviceLocator);

    void update(sf::Time dt) override;

private:
    void handleStoppedState(TrainMovementComponent &movement, float timeStep);
    
    void handleMovement(entt::entity entity, TrainMovementComponent &movement, TrainPhysicsComponent &physics, PositionComponent &position, float timeStep);
    
    void handleStationApproach(entt::entity entity, TrainMovementComponent &movement, TrainPhysicsComponent &physics, PositionComponent &position, StationApproachComponent &approach, float timeStep);

    entt::registry &_registry;
};