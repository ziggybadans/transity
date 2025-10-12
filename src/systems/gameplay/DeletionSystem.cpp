#include "DeletionSystem.h"
#include "components/LineComponents.h"
#include "components/GameLogicComponents.h"
#include "components/PassengerComponents.h"
#include "components/TrainComponents.h"
#include "Logger.h"
#include "core/Pathfinder.h"
#include <vector>
#include <algorithm>

DeletionSystem::DeletionSystem(entt::registry& registry, EventBus& eventBus, GameState& gameState)
    : _registry(registry), _eventBus(eventBus), _gameState(gameState) {
    _deleteEntityConnection = _eventBus.sink<DeleteEntityEvent>().connect<&DeletionSystem::onDeleteEntity>(this);
    _deleteAllEntitiesConnection = _eventBus.sink<DeleteAllEntitiesEvent>().connect<&DeletionSystem::onDeleteAllEntities>(this);
}

DeletionSystem::~DeletionSystem() {
    _eventBus.sink<DeleteEntityEvent>().disconnect(this);
    _eventBus.sink<DeleteAllEntitiesEvent>().disconnect(this);
}

void DeletionSystem::onDeleteEntity(const DeleteEntityEvent& event) {
    if (!_registry.valid(event.entity)) {
        LOG_WARN("DeletionSystem", "Attempted to delete an invalid entity: %u", entt::to_integral(event.entity));
        return;
    }

    // If the deleted entity is a line, trigger the specific line deletion logic.
    if (_registry.all_of<LineComponent>(event.entity)) {
        handleLineDeletion(event.entity);
    }

    _registry.destroy(event.entity);
    LOG_DEBUG("DeletionSystem", "Deleted entity: %u", entt::to_integral(event.entity));

    // If the deleted entity was the selected entity, deselect it
    if (_gameState.selectedEntity == event.entity) {
        _gameState.selectedEntity = std::nullopt;
    }
}

void DeletionSystem::handleLineDeletion(entt::entity lineEntity) {
    LOG_DEBUG("DeletionSystem", "Handling deletion of line %u.", entt::to_integral(lineEntity));
    deleteTrainsOnLine(lineEntity);
    removeLineFromCities(lineEntity);
    repathPassengersAfterLineDeletion(lineEntity);
}

void DeletionSystem::deleteTrainsOnLine(entt::entity lineEntity) {
    auto trainView = _registry.view<TrainTag, TrainMovementComponent>();
    std::vector<entt::entity> trainsToDelete;
    for (auto trainEntity : trainView) {
        if (trainView.get<TrainMovementComponent>(trainEntity).assignedLine == lineEntity) {
            trainsToDelete.push_back(trainEntity);
        }
    }

    for (auto trainEntity : trainsToDelete) {
        // Reset passengers on the train before deleting it
        auto passengerView = _registry.view<PassengerComponent>();
        for (auto passengerEntity : passengerView) {
            auto& passenger = passengerView.get<PassengerComponent>(passengerEntity);
            if (passenger.currentContainer == trainEntity) {
                passenger.state = PassengerState::WAITING_FOR_TRAIN;
                passenger.currentContainer = passenger.originStation;
                LOG_DEBUG("DeletionSystem", "Reset passenger %u on deleted train %u.", entt::to_integral(passengerEntity), entt::to_integral(trainEntity));
            }
        }
        _registry.destroy(trainEntity);
        LOG_DEBUG("DeletionSystem", "Deleted train %u because its line was deleted.", entt::to_integral(trainEntity));
    }
}

void DeletionSystem::removeLineFromCities(entt::entity lineEntity) {
    if (!_registry.valid(lineEntity)) return;
    const auto* line = _registry.try_get<LineComponent>(lineEntity);
    if (!line) return;

    for (const auto& point : line->points) {
        if (point.type == LinePointType::STOP && _registry.valid(point.stationEntity)) {
            auto& city = _registry.get<CityComponent>(point.stationEntity);
            auto& connectedLines = city.connectedLines;
            connectedLines.erase(std::remove(connectedLines.begin(), connectedLines.end(), lineEntity), connectedLines.end());
            LOG_DEBUG("DeletionSystem", "Removed deleted line %u from city %u.", entt::to_integral(lineEntity), entt::to_integral(point.stationEntity));
        }
    }
}

void DeletionSystem::repathPassengersAfterLineDeletion(entt::entity lineEntity) {
    Pathfinder pathfinder(_registry);
    std::vector<entt::entity> passengersToDelete;
    auto passengerView = _registry.view<PassengerComponent, PathComponent>();

    for (auto passengerEntity : passengerView) {
        auto& passengerComp = passengerView.get<PassengerComponent>(passengerEntity);
        auto& pathComp = passengerView.get<PathComponent>(passengerEntity);

        bool usesDeletedLine = std::find(pathComp.nodes.begin(), pathComp.nodes.end(), lineEntity) != pathComp.nodes.end();

        if (usesDeletedLine) {
            auto newPath = pathfinder.findPath(passengerComp.originStation, passengerComp.destinationStation);

            if (!newPath.empty()) {
                pathComp.nodes = newPath;
                pathComp.currentNodeIndex = 0;
                passengerComp.state = PassengerState::WAITING_FOR_TRAIN;
                passengerComp.currentContainer = passengerComp.originStation;
                LOG_DEBUG("DeletionSystem", "Passenger %u repathed successfully.", entt::to_integral(passengerEntity));
            } else {
                passengersToDelete.push_back(passengerEntity);
                LOG_DEBUG("DeletionSystem", "Passenger %u marked for deletion, no alternative path.", entt::to_integral(passengerEntity));
            }
        }
    }

    for (auto passengerEntity : passengersToDelete) {
        if (_registry.valid(passengerEntity)) {
            _registry.destroy(passengerEntity);
            LOG_DEBUG("DeletionSystem", "Deleted passenger %u.", entt::to_integral(passengerEntity));
        }
    }
}

void DeletionSystem::onDeleteAllEntities(const DeleteAllEntitiesEvent& event) {
    auto& registry = _registry;

    // Deselect any selected entity
    if (_gameState.selectedEntity.has_value()) {
        _gameState.selectedEntity = std::nullopt;
    }

    // Delete all trains
    auto trains = registry.view<TrainTag>();
    registry.destroy(trains.begin(), trains.end());

    // Delete all lines
    auto lines = registry.view<LineComponent>();
    registry.destroy(lines.begin(), lines.end());

    // Delete all cities
    auto cities = registry.view<CityComponent>();
    registry.destroy(cities.begin(), cities.end());

    // Delete all passengers
    auto passengers = registry.view<PassengerComponent>();
    registry.destroy(passengers.begin(), passengers.end());

    LOG_INFO("DeletionSystem", "All entities have been deleted.");
}