// src/systems/InputManager.cpp
#include "InputManager.h"

// Handles key press events. If a key that has been bound is pressed, its corresponding callback is invoked.
void InputManager::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) { // Check if the event is a key press.
        auto it = keyBindings.find(event.key.code); // Look for the key in the keyBindings map.
        if (it != keyBindings.end()) { // If the key is found in the map.
            it->second(); // Call the associated callback function.
        }
    }
}

// Binds a key to a callback function. This associates the key with a specific action.
void InputManager::bindKey(sf::Keyboard::Key key, KeyCallback callback) {
    keyBindings[key] = callback; // Store the key and its callback in the keyBindings map.
}