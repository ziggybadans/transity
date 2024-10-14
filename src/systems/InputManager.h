// src/systems/InputManager.h
#pragma once
#include <SFML/Window.hpp>
#include <unordered_map>
#include <functional>

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