#include "DeletionSystem.h"
#include "components/GameLogicComponents.h"
#include "Logger.h"

DeletionSystem::DeletionSystem(ServiceLocator& serviceLocator)
    : _serviceLocator(serviceLocator), _registry(serviceLocator.registry) {
    _deleteEntityConnection = _serviceLocator.eventBus.sink<DeleteEntityEvent>().connect<&DeletionSystem::onDeleteEntity>(this);
}

DeletionSystem::~DeletionSystem() {
    _serviceLocator.eventBus.sink<DeleteEntityEvent>().disconnect(this);
}

void DeletionSystem::onDeleteEntity(const DeleteEntityEvent& event) {
    if (!_registry.valid(event.entity)) {
        LOG_WARN("DeletionSystem", "Attempted to delete an invalid entity: %u", entt::to_integral(event.entity));
        return;
    }

    // If the deleted entity is a line, we need to handle the trains on that line.
    if (auto* line = _registry.try_get<LineComponent>(event.entity)) {
        // Find all trains on this line and delete them
        auto trainView = _registry.view<TrainComponent>();
        for (auto trainEntity : trainView) {
            auto& train = trainView.get<TrainComponent>(trainEntity);
            if (train.assignedLine == event.entity) {
                _registry.destroy(trainEntity);
                LOG_DEBUG("DeletionSystem", "Deleted train %u because its line was deleted.", entt::to_integral(trainEntity));
            }
        }
    }

    _registry.destroy(event.entity);
    LOG_DEBUG("DeletionSystem", "Deleted entity: %u", entt::to_integral(event.entity));

    // If the deleted entity was the selected entity, deselect it
    if (_serviceLocator.gameState.selectedEntity == event.entity) {
        _serviceLocator.gameState.selectedEntity = std::nullopt;
    }
}