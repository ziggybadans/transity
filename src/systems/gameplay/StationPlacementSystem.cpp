#include "StationPlacementSystem.h"
#include "Logger.h"
#include "ecs/EntityFactory.h"
#include <string>

StationPlacementSystem::StationPlacementSystem(entt::registry& registry, EntityFactory& entityFactory, GameState& gameState, EventBus& eventBus)
    : _registry(registry), _entityFactory(entityFactory),
      _gameState(gameState) {

    m_mousePressConnection = eventBus.sink<MouseButtonPressedEvent>()
                                 .connect<&StationPlacementSystem::onMouseButtonPressed>(this);
  LOG_DEBUG("StationPlacementSystem", "StationPlacementSystem created and connected to EventBus.");
}

StationPlacementSystem::~StationPlacementSystem() {
    m_mousePressConnection.release();
    LOG_DEBUG("StationPlacementSystem",
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