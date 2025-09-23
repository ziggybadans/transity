// src/systems/gameplay/PassengerMovementSystem.cpp

#include "PassengerMovementSystem.h"
#include "components/GameLogicComponents.h"
#include "components/PassengerComponents.h"
#include "core/ServiceLocator.h"
#include "Logger.h"
#include <algorithm>

PassengerMovementSystem::PassengerMovementSystem(ServiceLocator& serviceLocator)
    : _registry(serviceLocator.registry) {
    LOG_DEBUG("PassengerMovementSystem", "PassengerMovementSystem created.");
}

void PassengerMovementSystem::update(sf::Time dt) {
    // Find all trains that are currently stopped.
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
    auto it = std::find(line.stops.begin(), line.stops.end(), currentStopEntity);
    if (it == line.stops.end()) {
        return false;
    }
    size_t currentStopIndexOnLine = std::distance(line.stops.begin(), it);

    if (movement.direction == TrainDirection::FORWARD) {
        if (currentStopIndexOnLine + 1 < line.stops.size() && line.stops[currentStopIndexOnLine + 1] == nextNodeInPath) {
            return true;
        }
    } else { // TrainDirection::BACKWARD
        if (currentStopIndexOnLine > 0 && line.stops[currentStopIndexOnLine - 1] == nextNodeInPath) {
            return true;
        }
    }
    return false;
}

void PassengerMovementSystem::alightPassengers(entt::entity trainEntity, const TrainMovementComponent& movement, TrainCapacityComponent& capacity) {
    if (!_registry.valid(movement.assignedLine)) return;
    const auto& line = _registry.get<LineComponent>(movement.assignedLine);
    if (movement.currentSegmentIndex >= line.stops.size()) return;
    entt::entity currentStopEntity = line.stops[movement.currentSegmentIndex];
    if (!_registry.valid(currentStopEntity)) return;

    // Find all passengers currently on this train.
    auto passengerView = _registry.view<PassengerComponent, PathComponent>();
    for (auto passengerEntity : passengerView) {
        auto& passenger = passengerView.get<PassengerComponent>(passengerEntity);
        if (passenger.currentContainer != trainEntity) {
            continue;
        }

        // Case 1: Reached final destination.
        if (passenger.destinationStation == currentStopEntity) {
            _registry.destroy(passengerEntity);
            capacity.currentLoad--;
            LOG_TRACE("PassengerMovementSystem", "Passenger reached destination.");
            continue;
        }

        // Case 2: Need to transfer to a different line.
        auto& path = passengerView.get<PathComponent>(passengerEntity);
        if (path.currentNodeIndex + 1 < path.nodes.size()) {
            entt::entity nextNodeInPath = path.nodes[path.currentNodeIndex + 1];
            
            if (!isTrainGoingToNextNode(movement, line, currentStopEntity, nextNodeInPath)) {
                passenger.state = PassengerState::WAITING_FOR_TRAIN;
                passenger.currentContainer = currentStopEntity; // Move passenger to the station.
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
    if (movement.currentSegmentIndex >= line.stops.size()) return;
    entt::entity currentStopEntity = line.stops[movement.currentSegmentIndex];
    if (!_registry.valid(currentStopEntity)) return;

    // Find all passengers waiting at the current station.
    auto passengerView = _registry.view<PassengerComponent, PathComponent>();
    for (auto passengerEntity : passengerView) {
        if (capacity.currentLoad >= capacity.capacity) break; // Train is full.

        auto& passenger = passengerView.get<PassengerComponent>(passengerEntity);
        if (passenger.currentContainer != currentStopEntity) {
            continue;
        }

        auto& path = passengerView.get<PathComponent>(passengerEntity);
        if (path.currentNodeIndex >= path.nodes.size() - 1) continue;

        entt::entity nextNodeInPath = path.nodes[path.currentNodeIndex + 1];
        
        if (isTrainGoingToNextNode(movement, line, currentStopEntity, nextNodeInPath)) {
            passenger.state = PassengerState::ON_TRAIN;
            passenger.currentContainer = trainEntity; // Move passenger to the train.
            capacity.currentLoad++;
            LOG_TRACE("PassengerMovementSystem", "Passenger boarded train.");
        }
    }
}