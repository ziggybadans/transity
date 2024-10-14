// src/states/MainMenuState.h
#pragma once
#include "../core/State.h"
#include "../systems/InputManager.h"
#include "Renderer.h"

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
    Renderer renderer;

    // Add UI elements like buttons here, such as start game, settings, or quit.
};