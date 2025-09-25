#include "PassengerMovementSystem.h"
#include "components/GameLogicComponents.h"
#include "components/PassengerComponents.h"
#include "Logger.h"
#include <algorithm>

PassengerMovementSystem::PassengerMovementSystem(entt::registry& registry)
    : _registry(registry) {
    LOG_DEBUG("PassengerMovementSystem", "PassengerMovementSystem created.");
}

void PassengerMovementSystem::update(sf::Time dt) {
    auto trainView = _registry.view<TrainTag, TrainMovementComponent, TrainCapacityComponent>();
    for (auto trainEntity : trainView) {
        auto& movement = trainView.get<TrainMovementComponent>(trainEntity);

        if (movement.state != TrainState::STOPPED) {
            continue;
        }
        
        auto& capacity = trainView.get<TrainCapacityComponent>(trainEntity);

        alightPassengers(trainEntity, movement, capacity);
        boardPassengers(trainEntity, movement, capacity);
    }
}

bool PassengerMovementSystem::isTrainGoingToNextNode(const TrainMovementComponent& movement, const LineComponent& line, entt::entity currentStopEntity, entt::entity nextNodeInPath) {
    auto it = std::find_if(line.points.begin(), line.points.end(), [&](const LinePoint& p) {
        return p.type == LinePointType::STOP && p.stationEntity == currentStopEntity;
    });

    if (it == line.points.end()) {
        return false;
    }

    size_t currentStopIndexOnLine = std::distance(line.points.begin(), it);

    if (movement.direction == TrainDirection::FORWARD) {
        for (size_t i = currentStopIndexOnLine + 1; i < line.points.size(); ++i) {
            if (line.points[i].type == LinePointType::STOP) {
                return line.points[i].stationEntity == nextNodeInPath;
            }
        }
    } else { // TrainDirection::BACKWARD
        for (int i = currentStopIndexOnLine - 1; i >= 0; --i) {
            if (line.points[i].type == LinePointType::STOP) {
                return line.points[i].stationEntity == nextNodeInPath;
            }
        }
    }
    return false;
}

void PassengerMovementSystem::alightPassengers(entt::entity trainEntity, const TrainMovementComponent& movement, TrainCapacityComponent& capacity) {
    if (!_registry.valid(movement.assignedLine)) return;
    const auto& line = _registry.get<LineComponent>(movement.assignedLine);
    if (movement.currentSegmentIndex >= line.points.size() || line.points[movement.currentSegmentIndex].type != LinePointType::STOP) return;
    
    entt::entity currentStopEntity = line.points[movement.currentSegmentIndex].stationEntity;
    if (!_registry.valid(currentStopEntity)) return;

    auto passengerView = _registry.view<PassengerComponent, PathComponent>();
    for (auto passengerEntity : passengerView) {
        auto& passenger = passengerView.get<PassengerComponent>(passengerEntity);
        if (passenger.currentContainer != trainEntity) {
            continue;
        }

        if (passenger.destinationStation == currentStopEntity) {
            _registry.destroy(passengerEntity);
            capacity.currentLoad--;
            LOG_TRACE("PassengerMovementSystem", "Passenger reached destination.");
            continue;
        }

        auto& path = passengerView.get<PathComponent>(passengerEntity);
        if (path.currentNodeIndex + 1 < path.nodes.size()) {
            entt::entity nextNodeInPath = path.nodes[path.currentNodeIndex + 1];
            
            if (!isTrainGoingToNextNode(movement, line, currentStopEntity, nextNodeInPath)) {
                passenger.state = PassengerState::WAITING_FOR_TRAIN;
                passenger.currentContainer = currentStopEntity;
                path.currentNodeIndex++; 
                capacity.currentLoad--;
                LOG_TRACE("PassengerMovementSystem", "Passenger alighted to transfer.");
            }
        }
    }
}

void PassengerMovementSystem::boardPassengers(entt::entity trainEntity, const TrainMovementComponent& movement, TrainCapacityComponent& capacity) {
    if (!_registry.valid(movement.assignedLine)) return;
    const auto& line = _registry.get<LineComponent>(movement.assignedLine);
    if (movement.currentSegmentIndex >= line.points.size() || line.points[movement.currentSegmentIndex].type != LinePointType::STOP) return;
    
    entt::entity currentStopEntity = line.points[movement.currentSegmentIndex].stationEntity;
    if (!_registry.valid(currentStopEntity)) return;

    auto passengerView = _registry.view<PassengerComponent, PathComponent>();
    for (auto passengerEntity : passengerView) {
        if (capacity.currentLoad >= capacity.capacity) break;

        auto& passenger = passengerView.get<PassengerComponent>(passengerEntity);
        if (passenger.currentContainer != currentStopEntity) {
            continue;
        }

        auto& path = passengerView.get<PathComponent>(passengerEntity);
        if (path.currentNodeIndex >= path.nodes.size() - 1) continue;

        entt::entity nextNodeInPath = path.nodes[path.currentNodeIndex + 1];
        
        if (isTrainGoingToNextNode(movement, line, currentStopEntity, nextNodeInPath)) {
            passenger.state = PassengerState::ON_TRAIN;
            passenger.currentContainer = trainEntity;
            capacity.currentLoad++;
            LOG_TRACE("PassengerMovementSystem", "Passenger boarded train.");
        }
    }
}