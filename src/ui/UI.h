#pragma once

#include "app/GameState.h"
#include "app/LoadingState.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

class UI {
public:
    UI(sf::RenderWindow &window, LoadingState &loadingState);
    ~UI();
    void initialize();
    void processEvent(const sf::Event &event);
    void update(sf::Time deltaTime, AppState appState);
    void renderFrame();
    void cleanupResources();

private:
    void drawLoadingScreen();

    sf::RenderWindow &_window;
    LoadingState &_loadingState;
};