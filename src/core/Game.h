// src/core/Game.h
#pragma once

// Include the SFML Graphics library for rendering
#include <SFML/Graphics.hpp>
// Include memory management utilities (for smart pointers)
#include <memory>
// Include the "State" header, which defines the game states
#include "State.h"
#include "../systems/ResourceManager.h"

// Game class definition
class Game {
public:
    // Constructor: Initializes the game
    Game();

    // Main game loop: Runs the game until it is closed
    void run();

    // Push a new state onto the stack
    // This allows adding a new game state without removing the current one
    void pushState(std::unique_ptr<State> state);

    // Pop the current state from the stack
    // This removes the current state and resumes the previous one
    void popState();

    // Replace the current state with a new one
    // This is useful when transitioning to a completely different game state
    void changeState(std::unique_ptr<State> state);

    // Get a reference to the RenderWindow
    // This allows other objects to draw onto the window
    sf::RenderWindow& getWindow();

private:
    // The main rendering window for the game
    sf::RenderWindow window;

    // A stack of unique pointers to State objects
    // This allows managing multiple game states, such as menus, gameplay, etc.
    std::vector<std::unique_ptr<State>> states;

    // The time interval for each frame (targeting 60 frames per second)
    const sf::Time TimePerFrame = sf::seconds(1.f / 60.f);

    ResourceManager<sf::Texture> textureManager;
    ResourceManager<sf::Font> fontManager;
};