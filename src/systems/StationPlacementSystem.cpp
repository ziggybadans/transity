#include "StationPlacementSystem.h"
#include "../core/ServiceLocator.h"
#include "../core/EntityFactory.h"
#include "../Logger.h"
#include <string>

// The constructor now pulls its dependencies from the ServiceLocator
StationPlacementSystem::StationPlacementSystem(ServiceLocator& serviceLocator)
    : _registry(serviceLocator.registry),
      _entityFactory(serviceLocator.entityFactory),
      _gameState(serviceLocator.gameState) {
    m_placeStationConnection = serviceLocator.eventBus->sink<TryPlaceStationEvent>().connect<&StationPlacementSystem::onTryPlaceStation>(this);
    LOG_INFO("StationPlacementSystem", "StationPlacementSystem created and connected to EventBus.");
}

StationPlacementSystem::~StationPlacementSystem() {
    m_placeStationConnection.release();
    LOG_INFO("StationPlacementSystem", "StationPlacementSystem destroyed and disconnected from EventBus.");
}

void StationPlacementSystem::onTryPlaceStation(const TryPlaceStationEvent& event) {
    if (_gameState->currentInteractionMode == InteractionMode::CREATE_STATION) {
        LOG_DEBUG("StationPlacementSystem", "Processing TryPlaceStationEvent at (%.1f, %.1f)", event.worldPosition.x, event.worldPosition.y);
        int nextStationId = static_cast<int>(_registry->storage<entt::entity>().size());
        _entityFactory->createEntity("station", event.worldPosition, "New Station " + std::to_string(nextStationId));
    }
}
