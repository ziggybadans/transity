#pragma once

#include "app/GameState.h"
#include "event/EventBus.h"
#include <SFML/Graphics/RenderWindow.hpp>

class InteractionUI {
public:
    InteractionUI(GameState &gameState, EventBus &eventBus, sf::RenderWindow &window);
    ~InteractionUI();

    void draw(size_t numberOfStationsInActiveLine, size_t numberOfPointsInActiveLine);

private:
    void drawInteractionModeWindow();
    void drawLineCreationWindow(size_t numStationsInActiveLine, size_t numPointsInActiveLine);
    void drawPassengerCreationWindow();

    GameState &_gameState;
    EventBus &_eventBus;
    sf::RenderWindow &_window;
};