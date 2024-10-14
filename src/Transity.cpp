// src/main.cpp
#include "core/Game.h"
#include "states/MainMenuState.h" // You'll create this state

int main() {
    Game game;
    game.pushState(std::make_unique<MainMenuState>(game));
    game.run();
    return 0;
}
