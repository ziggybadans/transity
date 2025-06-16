#include "StationPlacementSystem.h"
#include "../input/InputHandler.h"
#include "../core/EntityFactory.h"
#include "../Logger.h"
#include "entt/entt.hpp"
#include <string>

StationPlacementSystem::StationPlacementSystem(InputHandler& inputHandler, entt::registry& registry, EntityFactory& entityFactory)
    : _inputHandler(inputHandler), _registry(registry), _entityFactory(entityFactory) {
}

void StationPlacementSystem::update(InteractionMode mode) {
    for (const auto& command : _inputHandler.getCommands()) {
        if (command.type == InputEventType::TRY_PLACE_STATION) {
            if (mode == InteractionMode::CREATE_STATION) {
                LOG_DEBUG("StationPlacementSystem", "Processing TryPlaceStation command at (%.1f, %.1f)", command.data.worldPosition.x, command.data.worldPosition.y);
                int nextStationId = static_cast<int>(_registry.storage<entt::entity>().size());
                _entityFactory.createEntity("station", command.data.worldPosition, "New Station " + std::to_string(nextStationId));
            }
        }
    }
}
