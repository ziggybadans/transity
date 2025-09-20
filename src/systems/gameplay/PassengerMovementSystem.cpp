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
    auto trainView = _registry.view<TrainComponent, PositionComponent>();

    // Helper to check if a train is heading towards a specific next stop
    auto isTrainGoingToNextNode = [&](const TrainComponent& train, const LineComponent& line, entt::entity currentStopEntity, entt::entity nextNodeInPath) {
        auto it = std::find(line.stops.begin(), line.stops.end(), currentStopEntity);
        if (it == line.stops.end()) {
            return false;
        }
        size_t currentStopIndexOnLine = std::distance(line.stops.begin(), it);

        if (train.direction == TrainDirection::FORWARD) {
            if (currentStopIndexOnLine + 1 < line.stops.size() && line.stops[currentStopIndexOnLine + 1] == nextNodeInPath) {
                return true;
            }
        } else { // TrainDirection::BACKWARD
            if (currentStopIndexOnLine > 0 && line.stops[currentStopIndexOnLine - 1] == nextNodeInPath) {
                return true;
            }
        }
        return false;
    };

    for (auto trainEntity : trainView) {
        auto& train = trainView.get<TrainComponent>(trainEntity);

        if (train.state != TrainState::STOPPED) {
            continue;
        }

        if (!_registry.valid(train.assignedLine)) continue;
        const auto& line = _registry.get<LineComponent>(train.assignedLine);
        if (train.currentSegmentIndex >= line.stops.size()) continue;
        entt::entity currentStopEntity = line.stops[train.currentSegmentIndex];
        if (!_registry.valid(currentStopEntity)) continue;

        // Alighting passengers
        train.passengers.erase(std::remove_if(train.passengers.begin(), train.passengers.end(),
            [&](entt::entity passengerEntity) {
                if (!_registry.valid(passengerEntity)) return true;

                auto& passenger = _registry.get<PassengerComponent>(passengerEntity);
                auto* path = _registry.try_get<PathComponent>(passengerEntity);

                if (passenger.destinationStation == currentStopEntity) {
                    _registry.destroy(passengerEntity);
                    train.currentLoad--;
                    LOG_DEBUG("PassengerMovementSystem", "Passenger reached destination.");
                    return true;
                }

                if (path && path->currentNodeIndex + 1 < path->nodes.size()) {
                    entt::entity nextNodeInPath = path->nodes[path->currentNodeIndex + 1];
                    
                    if (!isTrainGoingToNextNode(train, line, currentStopEntity, nextNodeInPath)) {
                        passenger.state = PassengerState::WAITING_FOR_TRAIN;
                        passenger.currentTrain = std::nullopt; // Clear the train
                        path->currentNodeIndex++; 
                        
                        if (_registry.all_of<CityComponent>(currentStopEntity)) {
                            auto& city = _registry.get<CityComponent>(currentStopEntity);
                            city.waitingPassengers.push_back(passengerEntity);
                        }
                        
                        train.currentLoad--;
                        LOG_DEBUG("PassengerMovementSystem", "Passenger alighted to transfer.");
                        return true; 
                    }
                }
                return false;
            }), train.passengers.end());

        // Boarding passengers
        if (_registry.all_of<CityComponent>(currentStopEntity)) {
            auto& city = _registry.get<CityComponent>(currentStopEntity);
            city.waitingPassengers.erase(std::remove_if(city.waitingPassengers.begin(), city.waitingPassengers.end(),
                [&](entt::entity passengerEntity) {
                    if (train.currentLoad >= train.capacity) return false;
                    if (!_registry.valid(passengerEntity)) return true;

                    auto* path = _registry.try_get<PathComponent>(passengerEntity);
                    if (!path || path->currentNodeIndex >= path->nodes.size() -1) return false;

                    entt::entity nextNodeInPath = path->nodes[path->currentNodeIndex + 1];
                    
                    if (isTrainGoingToNextNode(train, line, currentStopEntity, nextNodeInPath)) {
                        train.passengers.push_back(passengerEntity);
                        train.currentLoad++;
                        auto& passenger = _registry.get<PassengerComponent>(passengerEntity);
                        passenger.state = PassengerState::ON_TRAIN;
                        passenger.currentTrain = trainEntity; // Set the train
                        LOG_DEBUG("PassengerMovementSystem", "Passenger boarded train.");
                        return true;
                    }

                    return false;
                }), city.waitingPassengers.end());
        }
    }
}