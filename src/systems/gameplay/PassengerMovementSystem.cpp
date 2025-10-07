#include "PassengerMovementSystem.h"
#include "components/GameLogicComponents.h"
#include "components/PassengerComponents.h"
#include "Logger.h"
#include <algorithm>
#include <vector>

PassengerMovementSystem::PassengerMovementSystem(entt::registry& registry)
    : _registry(registry) {
    LOG_DEBUG("PassengerMovementSystem", "PassengerMovementSystem created.");
}

void PassengerMovementSystem::update(sf::Time dt) {
    // Iterate over trains that are currently at a station
    auto view = _registry.view<AtStationComponent>();
    for (auto trainEntity : view) {
        const auto& atStation = view.get<AtStationComponent>(trainEntity);
        entt::entity stationEntity = atStation.stationEntity;

        // Process alighting first to free up capacity
        alightPassengers(trainEntity, stationEntity);
        
        // Then process boarding with the newly available capacity
        boardPassengers(trainEntity, stationEntity);
    }
}

void PassengerMovementSystem::alightPassengers(entt::entity trainEntity, entt::entity stationEntity) {
    auto& capacity = _registry.get<TrainCapacityComponent>(trainEntity);
    const auto& movement = _registry.get<TrainMovementComponent>(trainEntity);
    
    if (!_registry.valid(movement.assignedLine)) return;
    const auto& line = _registry.get<LineComponent>(movement.assignedLine);

    // Iterate over a copy, as we modify components which can invalidate views
    std::vector<entt::entity> passengersOnTrain;
    auto passengerView = _registry.view<PassengerComponent>();
    for (auto passengerEntity : passengerView) {
        if (passengerView.get<PassengerComponent>(passengerEntity).currentContainer == trainEntity) {
            passengersOnTrain.push_back(passengerEntity);
        }
    }

    for (auto passengerEntity : passengersOnTrain) {
        if (!_registry.valid(passengerEntity)) continue; // Passenger might have been destroyed
        
        auto& passenger = _registry.get<PassengerComponent>(passengerEntity);
        auto& path = _registry.get<PathComponent>(passengerEntity);

        if (path.currentNodeIndex >= path.nodes.size()) {
            continue; // Path is already complete.
        }

        entt::entity nextStopOnPath = path.nodes[path.currentNodeIndex];

        // Alight only if the current station is the passenger's next destination.
        if (nextStopOnPath == stationEntity) {
            passenger.state = PassengerState::WAITING_FOR_TRAIN;
            passenger.currentContainer = stationEntity;
            capacity.currentLoad--;
            path.currentNodeIndex++; // CRITICAL: Progress the path to the *next* node
            LOG_TRACE("PassengerMovementSystem", "Passenger alighted at a path node.");

            // If the path is now complete, the passenger has arrived at their final destination.
            if (path.currentNodeIndex == path.nodes.size()) {
                _registry.destroy(passengerEntity);
                LOG_TRACE("PassengerMovementSystem", "Passenger reached final destination.");
            }
        }
    }
}

void PassengerMovementSystem::boardPassengers(entt::entity trainEntity, entt::entity stationEntity) {
    auto& capacity = _registry.get<TrainCapacityComponent>(trainEntity);
    if (capacity.currentLoad >= capacity.capacity) {
        return; // Train is full
    }

    const auto& movement = _registry.get<TrainMovementComponent>(trainEntity);

    if (!_registry.valid(movement.assignedLine)) return;
    const auto& line = _registry.get<LineComponent>(movement.assignedLine);

    // Find passengers waiting at this station
    auto passengerView = _registry.view<PassengerComponent, PathComponent>();
    for (auto passengerEntity : passengerView) {
        if (capacity.currentLoad >= capacity.capacity) {
            break; // Train became full during this loop
        }

        auto& passenger = passengerView.get<PassengerComponent>(passengerEntity);
        if (passenger.currentContainer != stationEntity || passenger.state != PassengerState::WAITING_FOR_TRAIN) {
            continue;
        }

        auto& path = passengerView.get<PathComponent>(passengerEntity);
        if (path.currentNodeIndex >= path.nodes.size()) {
            continue; // Path is complete, should not be waiting.
        }

        entt::entity nextNodeInPath = path.nodes[path.currentNodeIndex];

        // Check if this train is going towards the passenger's next destination
        if (isTrainGoingToNextNode(movement, line, stationEntity, nextNodeInPath)) {
            passenger.state = PassengerState::ON_TRAIN;
            passenger.currentContainer = trainEntity;
            capacity.currentLoad++;
            LOG_TRACE("PassengerMovementSystem", "Passenger boarded train.");
        }
    }
}

bool PassengerMovementSystem::isTrainGoingToNextNode(const TrainMovementComponent& movement, const LineComponent& line, entt::entity currentStopEntity, entt::entity nextNodeInPath) {
    // Find the index of the current stop on the line's point list
    auto it = std::find_if(line.points.begin(), line.points.end(), [&](const LinePoint& p) {
        return p.type == LinePointType::STOP && p.stationEntity == currentStopEntity;
    });

    if (it == line.points.end()) {
        return false;
    }
    size_t currentStopIndexOnLine = std::distance(line.points.begin(), it);

    // Search forward or backward from the current stop for the next node
    if (movement.direction == TrainDirection::FORWARD) {
        for (size_t i = currentStopIndexOnLine + 1; i < line.points.size(); ++i) {
            if (line.points[i].type == LinePointType::STOP) {
                if (line.points[i].stationEntity == nextNodeInPath) {
                    return true; // Found the next node in the forward direction
                }
            }
        }
    } else { // TrainDirection::BACKWARD
        for (int i = currentStopIndexOnLine - 1; i >= 0; --i) {
            if (line.points[i].type == LinePointType::STOP) {
                if (line.points[i].stationEntity == nextNodeInPath) {
                    return true; // Found the next node in the backward direction
                }
            }
        }
    }
    
    return false; // The next node is not on this line in the current direction
}