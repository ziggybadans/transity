// src/core/State.h
#pragma once
#include <SFML/Graphics.hpp>
#include "Game.h"

class State {
public:
    // Constructor: Initializes the state with a reference to the game instance
    State(Game& game) : game(game) {}
    virtual ~State() = default;

    // Pure virtual function to handle events (must be implemented by derived classes)
    virtual void handleEvent(const sf::Event& event) = 0;

    // Pure virtual function to update the game state (must be implemented by derived classes)
    virtual void update(sf::Time deltaTime) = 0;

    // Pure virtual function to render the state (must be implemented by derived classes)
    virtual void render() = 0;

protected:
    // Reference to the game instance for accessing shared resources
    Game& game;
};