// src/main.cpp
#include "core/Game.h"  // Includes the Game class definition
#include "systems/MainMenuState.h" // Includes the MainMenuState class definition, which will be implemented separately

int main() {
    Game game; // Create an instance of the Game class
    //game.pushState(std::make_unique<MainMenuState>(game)); // Add the MainMenuState to the game state stack using a unique pointer
    game.run(); // Start the game loop
    return 0; // End the program with a success status code
}

// Summary:
// This file is the entry point for the game application. It creates an instance of the Game class,
// pushes the initial game state (MainMenuState), and starts the game loop using the run() method.