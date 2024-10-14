// src/core/Game.cpp
#include "Game.h"
#include "../systems/MainMenuState.h"

// Constructor for the Game class. Initializes the game window with specific dimensions (800x600) and a title.
Game::Game() : window(sf::VideoMode(800, 600), "Transport Management Game") {}

// Main game loop. Responsible for running the game, updating states, and rendering.
void Game::run() {
    sf::Clock clock; // Keeps track of the passage of time.
    sf::Time timeSinceLastUpdate = sf::Time::Zero; // Time elapsed since the last update.

    while (window.isOpen()) { // Loop until the window is closed.
        sf::Time deltaTime = clock.restart(); // Restart the clock and get the time elapsed since the last restart.
        timeSinceLastUpdate += deltaTime;

        // Update the game state to match the desired frame rate (using TimePerFrame).
        while (timeSinceLastUpdate > TimePerFrame) {
            timeSinceLastUpdate -= TimePerFrame;

            sf::Event event;
            while (window.pollEvent(event)) { // Process all pending events.
                if (event.type == sf::Event::Closed) // If the close event is detected, close the window.
                    window.close();

                if (!states.empty())
                    states.back()->handleEvent(event); // Handle events for the current game state.
            }

            if (!states.empty())
                states.back()->update(TimePerFrame); // Update the current state.
        }

        if (!states.empty())
            states.back()->render(); // Render the current state.

        window.display(); // Display the rendered frame.
    }
}

// Adds a new state to the game by moving the passed state into the states stack.
void Game::pushState(std::unique_ptr<State> state) {
    states.emplace_back(std::move(state));
}

// Removes the current state from the stack if it is not empty.
void Game::popState() {
    if (!states.empty())
        states.pop_back();
}

// Changes the current game state by popping the current state and pushing a new one.
void Game::changeState(std::unique_ptr<State> state) {
    popState();
    pushState(std::move(state));
}

// Returns a reference to the main game window.
sf::RenderWindow& Game::getWindow() {
    return window;
}

// Summary:
// The Game class is responsible for managing the game loop, handling game states, and rendering. The main game
// loop ensures that the game runs at a consistent frame rate. The class maintains a stack of states, allowing
// for flexible transitions between different game states (e.g., menus, gameplay). State management is done using
// unique pointers to ensure that memory is handled correctly, preventing leaks.