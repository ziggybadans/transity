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
    // Add newly spawned passengers to their origin city's waiting list
    auto newPassengerView = _registry.view<PassengerComponent>();
    for (auto entity : newPassengerView) {
        auto& passenger = newPassengerView.get<PassengerComponent>(entity);
        if (passenger.state == PassengerState::WAITING_FOR_TRAIN) {
            if (_registry.valid(passenger.originStation) && _registry.all_of<CityComponent>(passenger.originStation)) {
                auto& city = _registry.get<CityComponent>(passenger.originStation);
                if (std::find(city.waitingPassengers.begin(), city.waitingPassengers.end(), entity) == city.waitingPassengers.end()) {
                    city.waitingPassengers.push_back(entity);
                }
            }
        }
    }

    auto trainView = _registry.view<TrainComponent>();
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
                if (!_registry.valid(passengerEntity)) return true; // Remove if invalid
                auto& passenger = _registry.get<PassengerComponent>(passengerEntity);
                if (passenger.destinationStation == currentStopEntity) {
                    _registry.destroy(passengerEntity);
                    train.currentLoad--;
                    LOG_DEBUG("PassengerMovementSystem", "Passenger alighted and reached destination.");
                    return true;
                }
                return false;
            }), train.passengers.end());

        // Boarding passengers
        if (_registry.all_of<CityComponent>(currentStopEntity)) {
            auto& city = _registry.get<CityComponent>(currentStopEntity);
            city.waitingPassengers.erase(std::remove_if(city.waitingPassengers.begin(), city.waitingPassengers.end(),
                [&](entt::entity passengerEntity) {
                    if (train.currentLoad >= train.capacity) {
                        return false; // Train is full
                    }
                    if (!_registry.valid(passengerEntity)) return true; // Remove if invalid

                    // For now, any stopped train is a valid option.
                    // A future pathfinding system would add more complex logic here.
                    train.passengers.push_back(passengerEntity);
                    train.currentLoad++;
                    
                    auto& passenger = _registry.get<PassengerComponent>(passengerEntity);
                    passenger.state = PassengerState::ON_TRAIN;
                    
                    LOG_DEBUG("PassengerMovementSystem", "Passenger boarded train.");
                    return true;
                }), city.waitingPassengers.end());
        }
    }
}