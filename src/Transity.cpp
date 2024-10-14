// src/main.cpp

#include "Game.h"

int main() {
    // Create a game instance with chunk size 10 and tile size 50 pixels
    Game game(10, 50);
    // Run the game loop
    game.run();
    return 0;
}