// Game.h
#pragma once
#include <SFML/Graphics.hpp>  // Include the SFML graphics library for rendering
#include <memory>             // Include for managing unique pointers
#include <vector>             // Include for storing the game states
#include "State.h"          // Include the base State class for game states

class Game {
public:
    Game();  // Constructor to initialize the game
    void run();  // Method to start the main game loop

    void pushState(std::unique_ptr<State> state);  // Add a new state to the stack
    void popState();  // Remove the top state from the stack
    void changeState(std::unique_ptr<State> state);  // Replace the current state with a new one

    sf::RenderWindow& getWindow();  // Accessor for the render window

private:
    sf::RenderWindow window;  // The window where the game is rendered
    std::vector<std::unique_ptr<State>> states;  // Stack of game states
    const sf::Time TimePerFrame = sf::seconds(1.f / 60.f);  // Fixed time step for frame updates, set to 60 FPS
};

// Summary:
// The Game class handles the overall management of the game, including maintaining the game state stack
// and running the main game loop. It uses SFML for rendering and manages states using unique pointers
// to ensure proper memory management.