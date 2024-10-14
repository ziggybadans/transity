// src/states/MainMenuState.cpp
#include "MainMenuState.h"

// Constructor for the MainMenuState. Binds keys for interactions in the main menu.
MainMenuState::MainMenuState(Game& game) : State(game) {
    inputManager.bindKey(sf::Keyboard::Enter, [this]() {
        // Transition to Gameplay State
        // Uncomment the following line to switch to the GameplayState when Enter is pressed.
        // game.changeState(std::make_unique<GameplayState>(game));
        });
    // Bind other keys as needed for menu interactions.
}

// Handles events for the main menu, such as key presses.
void MainMenuState::handleEvent(const sf::Event& event) {
    inputManager.handleEvent(event); // Delegate key press handling to InputManager.
    // Handle other events here (e.g., UI interactions like button clicks).
}

// Updates the state of the main menu, useful for animations or UI element changes.
void MainMenuState::update(sf::Time deltaTime) {
    // Update UI elements if needed (e.g., animations or button highlights).
}

// Renders the main menu screen, including drawing any UI elements.
void MainMenuState::render() {
    game.getWindow().clear(sf::Color::Blue);
    renderer.render(game.getWindow());
    game.getWindow().display();
}