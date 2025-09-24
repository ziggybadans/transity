#pragma once

#include "ecs/ISystem.h"
#include "event/InputEvents.h"
#include "app/GameState.h"
#include "event/EventBus.h"
#include <entt/entt.hpp>

class GameStateSystem : public ISystem, public IUpdatable {
public:
    explicit GameStateSystem(EventBus& eventBus, GameState& gameState);
    ~GameStateSystem();

    void update(sf::Time dt) override;

private:
    void onInteractionModeChange(const InteractionModeChangeEvent &event);

    EventBus& _eventBus;
    GameState& _gameState;
    entt::scoped_connection _interactionModeChangeListener;
};