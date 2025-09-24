#include "GameStateSystem.h"
#include "Logger.h"

GameStateSystem::GameStateSystem(EventBus& eventBus, GameState& gameState) 
    : _eventBus(eventBus), _gameState(gameState) {
    _interactionModeChangeListener = _eventBus.sink<InteractionModeChangeEvent>()
                                         .connect<&GameStateSystem::onInteractionModeChange>(this);
   LOG_DEBUG("GameStateSystem",
            "GameStateSystem created and listening for interaction mode changes.");
}

GameStateSystem::~GameStateSystem() {
    _eventBus.sink<InteractionModeChangeEvent>().disconnect(this);
    LOG_DEBUG("GameStateSystem", "GameStateSystem destroyed.");
}

void GameStateSystem::onInteractionModeChange(const InteractionModeChangeEvent &event) {
    _gameState.currentInteractionMode = event.newMode;
    LOG_DEBUG("GameStateSystem", "Interaction mode changed to: %d", static_cast<int>(event.newMode));
}

void GameStateSystem::update(sf::Time dt) {}