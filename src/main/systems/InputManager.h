// src/systems/InputManager.h
#pragma once
#include <SFML/Window.hpp>  // Include for handling SFML window events and keyboard input
#include <unordered_map>    // Include for storing key bindings efficiently
#include <functional>       // Include for using std::function as callbacks

class InputManager {
public:
    using KeyCallback = std::function<void()>; // Defines a type alias for a function callback that takes no arguments and returns void.

    // Handles SFML events. Used to trigger actions based on input events.
    void handleEvent(const sf::Event& event);

    // Binds a specific keyboard key to a callback function.
    void bindKey(sf::Keyboard::Key key, KeyCallback callback);

private:
    // Stores key bindings where each key is mapped to a corresponding callback function.
    std::unordered_map<sf::Keyboard::Key, KeyCallback> keyBindings;
};

// Summary:
// The InputManager class provides functionality for managing user input via keyboard keys. It allows for the binding of specific
// keys to callback functions, enabling custom actions to be performed when keys are pressed. The handleEvent method processes
// SFML events and triggers the appropriate callback if a registered key event occurs. This design makes handling input simple
// and decouples input logic from the rest of the game systems.