#include "StationPlacementSystem.h"
#include "../input/InputHandler.h"
#include "../core/EntityFactory.h"
#include "../Logger.h"
#include "entt/entt.hpp"
#include <string>

// The implementation now uses the passed-in mode
void StationPlacementSystem::update(InputHandler& inputHandler, InteractionMode mode, entt::registry& registry, EntityFactory& entityFactory) {
    for (const auto& command : inputHandler.getCommands()) {
        if (command.type == InputEventType::TRY_PLACE_STATION) {
            if (mode == InteractionMode::CREATE_STATION) {
                LOG_DEBUG("StationPlacementSystem", "Processing TryPlaceStation command at (%.1f, %.1f)", command.data.worldPosition.x, command.data.worldPosition.y);
                int nextStationId = static_cast<int>(registry.storage<entt::entity>().size());
                entityFactory.createEntity("station", command.data.worldPosition, "New Station " + std::to_string(nextStationId));
            }
        }
    }
}
