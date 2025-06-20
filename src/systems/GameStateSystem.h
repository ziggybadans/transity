// src/systems/GameStateSystem.h
#pragma once

#include "../core/ServiceLocator.h"
#include "../event/InputEvents.h"
#include "../core/ISystem.h"

class GameStateSystem : public ISystem {
public:
    explicit GameStateSystem(ServiceLocator& services);
    ~GameStateSystem();

    void update(sf::Time dt) override;

private:
    void onInteractionModeChange(const InteractionModeChangeEvent& event);

    ServiceLocator& _services;
    entt::connection _interactionModeChangeListener;
};
