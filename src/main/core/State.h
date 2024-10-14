// src/core/State.h
#pragma once
#include <SFML/Graphics.hpp>  // Include SFML graphics to handle rendering and events

class Game;  // Forward declaration of the Game class to avoid circular dependency

class State {
public:
    // Constructor: Initializes the state with a reference to the game instance
    State(Game& game) : game(game) {}
    virtual ~State() = default;  // Virtual destructor to ensure derived class destructors are called properly

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

// Summary:
// The State class serves as an abstract base class for different game states, such as menus or gameplay scenes.
// It requires derived classes to implement event handling, updating, and rendering methods. The State class holds
// a reference to the main Game object, allowing it to interact with shared resources and other game elements.