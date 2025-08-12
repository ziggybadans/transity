
#include "GameStateSystem.h"
#include "../Logger.h"
#include "../core/GameState.h"

GameStateSystem::GameStateSystem(ServiceLocator &services) : _services(services) {
    _interactionModeChangeListener = _services.eventBus->sink<InteractionModeChangeEvent>()
                                         .connect<&GameStateSystem::onInteractionModeChange>(this);
    LOG_INFO("GameStateSystem",
             "GameStateSystem created and listening for interaction mode changes.");
}

GameStateSystem::~GameStateSystem() {
    _services.eventBus->sink<InteractionModeChangeEvent>().disconnect(this);
    LOG_INFO("GameStateSystem", "GameStateSystem destroyed.");
}

void GameStateSystem::onInteractionModeChange(const InteractionModeChangeEvent &event) {
    if (_services.gameState) {
        _services.gameState->currentInteractionMode = event.newMode;
        LOG_INFO("GameStateSystem", "Interaction mode changed to: %d",
                 static_cast<int>(event.newMode));
    }
}

void GameStateSystem::update(sf::Time dt) {
    
}
