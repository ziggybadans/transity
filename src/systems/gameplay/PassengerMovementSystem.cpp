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
    
    entt::entity currentStopEntity = getCurrentStop(movement, line);
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
    
    entt::entity currentStopEntity = getCurrentStop(movement, line);
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

entt::entity PassengerMovementSystem::getCurrentStop(const TrainMovementComponent& movement, const LineComponent& line) {
    if (line.points.empty()) {
        return entt::null;
    }

    float minDistance = -1.0f;
    entt::entity closestStop = entt::null;

    float currentCurveDistance = 0.0f;
    size_t nextPointIndex = 0;
    for(size_t i = 0; i < line.curvePoints.size(); ++i) {
        if (nextPointIndex < line.points.size()) {
            const auto& p = line.points[nextPointIndex];
            if (line.curvePoints[i] == p.position) {
                if (p.type == LinePointType::STOP) {
                    float distanceToTrain = std::abs(currentCurveDistance - movement.distanceAlongCurve);
                    if (closestStop == entt::null || distanceToTrain < minDistance) {
                        minDistance = distanceToTrain;
                        closestStop = p.stationEntity;
                    }
                }
                nextPointIndex++;
            }
        }
        if (i < line.curvePoints.size() - 1) {
            const auto& p1 = line.curvePoints[i];
            const auto& p2 = line.curvePoints[i + 1];
            currentCurveDistance += std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
        }
    }
    return closestStop;
}