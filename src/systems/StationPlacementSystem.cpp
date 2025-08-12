// src/systems/StationPlacementSystem.cpp

#include "StationPlacementSystem.h"
#include "../Logger.h"
#include "../core/EntityFactory.h"
#include "../core/ServiceLocator.h"
#include <string>

StationPlacementSystem::StationPlacementSystem(ServiceLocator &serviceLocator)
    : _registry(serviceLocator.registry), _entityFactory(serviceLocator.entityFactory),
      _gameState(serviceLocator.gameState) {
    // Connect to the new generic mouse press event
    m_mousePressConnection = serviceLocator.eventBus->sink<MouseButtonPressedEvent>()
                                 .connect<&StationPlacementSystem::onMouseButtonPressed>(this);
    LOG_INFO("StationPlacementSystem", "StationPlacementSystem created and connected to EventBus.");
}

StationPlacementSystem::~StationPlacementSystem() {
    // Release the new connection
    m_mousePressConnection.release();
    LOG_INFO("StationPlacementSystem",
             "StationPlacementSystem destroyed and disconnected from EventBus.");
}

// New handler implementation
void StationPlacementSystem::onMouseButtonPressed(const MouseButtonPressedEvent &event) {
    // Logic moved from InputHandler: check mode and button
    if (_gameState->currentInteractionMode == InteractionMode::CREATE_STATION
        && event.button == sf::Mouse::Button::Right) {
        LOG_DEBUG("StationPlacementSystem", "Processing MouseButtonPressedEvent at (%.1f, %.1f)",
                  event.worldPosition.x, event.worldPosition.y);
        int nextStationId = static_cast<int>(_registry->storage<entt::entity>().size());
        _entityFactory->createEntity("station", event.worldPosition,
                                     "New Station " + std::to_string(nextStationId));
    }
}

void StationPlacementSystem::update(sf::Time dt) {
    // This system is purely event-driven, so this can be empty.
}