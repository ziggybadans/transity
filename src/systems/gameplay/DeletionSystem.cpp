#include "DeletionSystem.h"
#include "components/GameLogicComponents.h"
#include "Logger.h"

DeletionSystem::DeletionSystem(entt::registry& registry, EventBus& eventBus, GameState& gameState)
    : _registry(registry), _eventBus(eventBus), _gameState(gameState) {
    _deleteEntityConnection = _eventBus.sink<DeleteEntityEvent>().connect<&DeletionSystem::onDeleteEntity>(this);
}

DeletionSystem::~DeletionSystem() {
    _eventBus.sink<DeleteEntityEvent>().disconnect(this);
}

void DeletionSystem::onDeleteEntity(const DeleteEntityEvent& event) {
    if (!_registry.valid(event.entity)) {
        LOG_WARN("DeletionSystem", "Attempted to delete an invalid entity: %u", entt::to_integral(event.entity));
        return;
    }

    // If the deleted entity is a line, we need to handle the trains on that line.
    if (auto* line = _registry.try_get<LineComponent>(event.entity)) {
        // Find all trains on this line and delete them
        auto trainView = _registry.view<TrainTag, TrainMovementComponent>();
        for (auto trainEntity : trainView) {
            auto& movement = trainView.get<TrainMovementComponent>(trainEntity);
            if (movement.assignedLine == event.entity) {
                _registry.destroy(trainEntity);
                LOG_DEBUG("DeletionSystem", "Deleted train %u because its line was deleted.", entt::to_integral(trainEntity));
            }
        }
    }

    _registry.destroy(event.entity);
    LOG_DEBUG("DeletionSystem", "Deleted entity: %u", entt::to_integral(event.entity));

    // If the deleted entity was the selected entity, deselect it
    if (_gameState.selectedEntity == event.entity) {
        _gameState.selectedEntity = std::nullopt;
    }
}