

#include "StationPlacementSystem.h"
#include "Logger.h"
#include "core/ServiceLocator.h"
#include "ecs/EntityFactory.h"
#include <string>

StationPlacementSystem::StationPlacementSystem(ServiceLocator &serviceLocator)
    : _registry(serviceLocator.registry), _entityFactory(serviceLocator.entityFactory),
      _gameState(serviceLocator.gameState) {

    m_mousePressConnection = serviceLocator.eventBus.sink<MouseButtonPressedEvent>()
                                 .connect<&StationPlacementSystem::onMouseButtonPressed>(this);
    LOG_INFO("StationPlacementSystem", "StationPlacementSystem created and connected to EventBus.");
}

StationPlacementSystem::~StationPlacementSystem() {

    m_mousePressConnection.release();
    LOG_INFO("StationPlacementSystem",
             "StationPlacementSystem destroyed and disconnected from EventBus.");
}

void StationPlacementSystem::onMouseButtonPressed(const MouseButtonPressedEvent &event) {

    if (_gameState.currentInteractionMode == InteractionMode::CREATE_STATION
        && event.button == sf::Mouse::Button::Right) {
        LOG_DEBUG("StationPlacementSystem", "Processing MouseButtonPressedEvent at (%.1f, %.1f)",
                  event.worldPosition.x, event.worldPosition.y);
        int nextStationId = static_cast<int>(_registry.storage<entt::entity>().size());
        _entityFactory.createEntity("station", event.worldPosition,
                                    "New Station " + std::to_string(nextStationId));
    }
}

void StationPlacementSystem::update(sf::Time dt) {}