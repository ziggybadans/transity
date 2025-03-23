#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <unordered_map>
#include <queue>
#include <memory>

namespace transity::core {

class InputManager {
public:
    static InputManager& getInstance();

    // Delete copy constructor and assignment operator
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    // Event handling
    void processEvent(const sf::Event& event);
    void update();
    void clear();

    // Keyboard state queries
    bool isKeyPressed(sf::Keyboard::Key key) const;
    bool isKeyJustPressed(sf::Keyboard::Key key) const;
    bool isKeyJustReleased(sf::Keyboard::Key key) const;

    // Mouse state queries
    bool isMouseButtonPressed(sf::Mouse::Button button) const;
    bool isMouseButtonJustPressed(sf::Mouse::Button button) const;
    bool isMouseButtonJustReleased(sf::Mouse::Button button) const;
    
    sf::Vector2i getMousePosition() const;
    sf::Vector2i getMouseDelta() const;
    float getMouseWheelDelta() const;

private:
    InputManager() = default;  // Private constructor for singleton

    // Keyboard state tracking
    std::unordered_map<sf::Keyboard::Key, bool> currentKeyStates;
    std::unordered_map<sf::Keyboard::Key, bool> previousKeyStates;

    // Mouse state tracking
    std::unordered_map<sf::Mouse::Button, bool> currentMouseStates;
    std::unordered_map<sf::Mouse::Button, bool> previousMouseStates;
    
    sf::Vector2i currentMousePos;
    sf::Vector2i previousMousePos;
    sf::Vector2i mouseDelta;
    float mouseWheelDelta = 0.0f;
}; // End of class InputManager
} // End of namespace transity::core 