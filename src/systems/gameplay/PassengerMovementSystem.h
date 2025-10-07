#pragma once

#include "ecs/ISystem.h"
#include <SFML/System/Time.hpp>
#include <entt/entt.hpp>

struct LineComponent;
struct TrainMovementComponent;

class PassengerMovementSystem : public ISystem, public IUpdatable {
public:
    explicit PassengerMovementSystem(entt::registry& registry);

    void update(sf::Time dt) override;

private:
    void alightPassengers(entt::entity trainEntity, entt::entity stationEntity);
    void boardPassengers(entt::entity trainEntity, entt::entity stationEntity);

    bool isTrainGoingToNextNode(const TrainMovementComponent& movement, const LineComponent& line, entt::entity currentStopEntity, entt::entity nextNodeInPath);

    entt::registry& _registry;
};