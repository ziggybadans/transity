#pragma once

#include <SFML/Graphics.hpp>
#include "Game.h" // For InteractionMode

// Forward declarations
namespace sf {
    class RenderWindow;
    class Event;
}

class UIManager {
public:
    UIManager();
    ~UIManager();

    void init(sf::RenderWindow& window); // Keep as reference for init
    void processEvent(const sf::Event& event);
    void update(); // Will use stored window and clock
    void render();
    InteractionMode getCurrentInteractionMode() const;
    void shutdown();

private:
    InteractionMode m_currentInteractionMode;
    sf::RenderWindow* m_window; // Store a pointer to the window
    sf::Clock m_deltaClock;     // For ImGui delta time
    // Add other necessary private members, e.g., for ImGui state
};