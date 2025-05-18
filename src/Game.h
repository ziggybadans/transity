#pragma once

#include <SFML/Graphics.hpp> // For sf::RenderWindow, sf::Time
#include <SFML/Window.hpp>   // For sf::Event
#include <SFML/System.hpp>   // For sf::Clock
#include <optional>          // Keep if used elsewhere, or remove if only for old event handling
#include "Camera.h"
#include <entt/entt.hpp>
#include "Renderer.h"        // Include the new Renderer header
#include <memory>            // For std::unique_ptr

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(sf::Time dt);
    // render() is now handled by m_renderer

    sf::RenderWindow window; // Game owns the window
    Camera camera;
    sf::Clock deltaClock;
    // sf::RectangleShape land; // Moved to Renderer
    // sf::Color oceanColor;   // Moved to Renderer

    entt::registry registry;
    std::unique_ptr<Renderer> m_renderer; // Renderer instance
};