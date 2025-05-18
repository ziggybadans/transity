#pragma once

#include <SFML/Graphics.hpp>

// Forward declarations
namespace entt {
    class registry;
}
class Camera;

class Renderer {
public:
    // Constructor takes a reference to the window created and owned by Game
    Renderer(sf::RenderWindow& window);
    ~Renderer();

    // Initialize renderer-specific settings (e.g., clear color, land shape)
    void init();

    // Main render function, needs game state (registry) and camera view
    void render(entt::registry& registry, Camera& camera);

    // Clean up rendering resources
    void cleanup();

    // Method to set the clear color (optional, can be set in init)
    void setClearColor(const sf::Color& color);
    const sf::Color& getClearColor() const;

private:
    sf::RenderWindow& m_window;     // Reference to the Game's window
    sf::Color m_clearColor;         // Formerly oceanColor
    sf::RectangleShape m_landShape;  // Formerly Game::land
};