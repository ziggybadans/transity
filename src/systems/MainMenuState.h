// src/states/MainMenuState.h
#pragma once
#include "../core/State.h"  // Include the base State class for the game states
#include "../systems/InputManager.h"  // Include the InputManager to manage input for the menu
#include "Renderer.h"  // Include the Renderer to handle rendering UI elements

class MainMenuState : public State {
public:
    // Constructor that initializes the MainMenuState with a reference to the game instance.
    MainMenuState(Game& game);

    // Handles input events specific to the main menu (e.g., button clicks).
    void handleEvent(const sf::Event& event) override;

    // Updates the main menu state, typically for animations or button interactions.
    void update(sf::Time deltaTime) override;

    // Renders the main menu state, including drawing UI elements.
    void render() override;

private:
    InputManager inputManager; // Manages input for the main menu (e.g., key bindings for navigating the menu).
    Renderer renderer; // Handles the rendering of the main menu elements such as buttons or backgrounds.

    // Add UI elements like buttons here, such as start game, settings, or quit.
};

// Summary:
// The MainMenuState class defines the behavior for the main menu in the game. It inherits from the base State class and implements
// specific methods to handle input, update animations or interactions, and render the main menu UI. This class utilizes the
// InputManager to process user input and the Renderer to draw the menu elements, providing a clean interface for transitioning
// into the game or settings.