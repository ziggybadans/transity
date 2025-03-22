#include "transity/core/input_manager.hpp"

namespace transity {
namespace core {

InputManager& InputManager::getInstance() {
    static InputManager instance;
    return instance;
}

void InputManager::processEvent(const sf::Event& event) {
    switch (event.type) {
        case sf::Event::KeyPressed:
            currentKeyStates[event.key.code] = true;
            break;
        case sf::Event::KeyReleased:
            currentKeyStates[event.key.code] = false;
            break;
        case sf::Event::MouseButtonPressed:
            currentMouseStates[event.mouseButton.button] = true;
            break;
        case sf::Event::MouseButtonReleased:
            currentMouseStates[event.mouseButton.button] = false;
            break;
        case sf::Event::MouseMoved:
            currentMousePos = {event.mouseMove.x, event.mouseMove.y};
            break;
        case sf::Event::MouseWheelScrolled:
            mouseWheelDelta = event.mouseWheelScroll.delta;
            break;
        default:
            break;
    }
}

void InputManager::update() {
    // Update mouse delta
    mouseDelta = {
        currentMousePos.x - previousMousePos.x,
        currentMousePos.y - previousMousePos.y
    };

    // Store previous states
    previousKeyStates = currentKeyStates;
    previousMouseStates = currentMouseStates;
    previousMousePos = currentMousePos;
}

void InputManager::clear() {
    mouseWheelDelta = 0.0f;
}

bool InputManager::isKeyPressed(sf::Keyboard::Key key) const {
    auto it = currentKeyStates.find(key);
    return it != currentKeyStates.end() && it->second;
}

bool InputManager::isKeyJustPressed(sf::Keyboard::Key key) const {
    auto currentIt = currentKeyStates.find(key);
    auto previousIt = previousKeyStates.find(key);
    
    bool currentState = (currentIt != currentKeyStates.end()) && currentIt->second;
    bool previousState = (previousIt != previousKeyStates.end()) && previousIt->second;
    
    return currentState && !previousState;
}

bool InputManager::isKeyJustReleased(sf::Keyboard::Key key) const {
    auto currentIt = currentKeyStates.find(key);
    auto previousIt = previousKeyStates.find(key);
    
    bool currentState = (currentIt != currentKeyStates.end()) && currentIt->second;
    bool previousState = (previousIt != previousKeyStates.end()) && previousIt->second;
    
    return !currentState && previousState;
}

bool InputManager::isMouseButtonPressed(sf::Mouse::Button button) const {
    auto it = currentMouseStates.find(button);
    return it != currentMouseStates.end() && it->second;
}

bool InputManager::isMouseButtonJustPressed(sf::Mouse::Button button) const {
    auto currentIt = currentMouseStates.find(button);
    auto previousIt = previousMouseStates.find(button);
    
    bool currentState = (currentIt != currentMouseStates.end()) && currentIt->second;
    bool previousState = (previousIt != previousMouseStates.end()) && previousIt->second;
    
    return currentState && !previousState;
}

bool InputManager::isMouseButtonJustReleased(sf::Mouse::Button button) const {
    auto currentIt = currentMouseStates.find(button);
    auto previousIt = previousMouseStates.find(button);
    
    bool currentState = (currentIt != currentMouseStates.end()) && currentIt->second;
    bool previousState = (previousIt != previousMouseStates.end()) && previousIt->second;
    
    return !currentState && previousState;
}

sf::Vector2i InputManager::getMousePosition() const {
    return currentMousePos;
}

sf::Vector2i InputManager::getMouseDelta() const {
    return mouseDelta;
}

float InputManager::getMouseWheelDelta() const {
    return mouseWheelDelta;
}

} // namespace core
} // namespace transity 