#include "GameStateSystem.h"
#include "Logger.h"

GameStateSystem::GameStateSystem(EventBus& eventBus, GameState& gameState) 
    : _eventBus(eventBus), _gameState(gameState) {
    _interactionModeChangeListener = _eventBus.sink<InteractionModeChangeEvent>()
                                         .connect<&GameStateSystem::onInteractionModeChange>(this);
    _startPassengerCreationListener = _eventBus.sink<StartPassengerCreationEvent>()
                                          .connect<&GameStateSystem::onStartPassengerCreation>(this);
   LOG_DEBUG("GameStateSystem",
            "GameStateSystem created and listening for interaction mode changes.");
}

GameStateSystem::~GameStateSystem() {
    _eventBus.sink<InteractionModeChangeEvent>().disconnect(this);
    _eventBus.sink<StartPassengerCreationEvent>().disconnect(this);
    LOG_DEBUG("GameStateSystem", "GameStateSystem destroyed.");
}

void GameStateSystem::onInteractionModeChange(const InteractionModeChangeEvent &event) {
    _gameState.currentInteractionMode = event.newMode;
    LOG_DEBUG("GameStateSystem", "Interaction mode changed to: %d", static_cast<int>(event.newMode));
}

void GameStateSystem::onStartPassengerCreation(const StartPassengerCreationEvent &event) {
    _gameState.passengerOriginStation = event.originStation;
    _gameState.currentInteractionMode = InteractionMode::CREATE_PASSENGER;
    LOG_DEBUG("GameStateSystem", "Starting passenger creation from station %u", entt::to_integral(event.originStation));
}

void GameStateSystem::update(sf::Time dt) {}