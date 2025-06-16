#include "StationPlacementSystem.h"
#include "../input/InputHandler.h"
#include "../graphics/UI.h"
#include "../core/EntityFactory.h"
#include "../Logger.h"
#include "entt/entt.hpp" // Added to ensure full definition of entt::registry
#include <string>

void StationPlacementSystem::update(InputHandler& inputHandler, UI& ui, entt::registry& registry, EntityFactory& entityFactory) {
    for (const auto& command : inputHandler.getCommands()) {
        if (command.type == InputEventType::TRY_PLACE_STATION) {
            if (ui.getInteractionMode() == InteractionMode::CREATE_STATION) {
                LOG_DEBUG("StationPlacementSystem", "Processing TryPlaceStation command at (%.1f, %.1f)", command.data.worldPosition.x, command.data.worldPosition.y);
                int nextStationId = static_cast<int>(registry.storage<entt::entity>().size());
                entityFactory.createStation(command.data.worldPosition, "New Station " + std::to_string(nextStationId));
            }
        }
    }
}