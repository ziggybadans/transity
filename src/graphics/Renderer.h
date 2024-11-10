// Renderer.h
#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <mutex>
#include "Camera.h"
#include "../world/WorldMap.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Initialize the renderer
    bool Init(sf::RenderWindow& window);

    // Set the WorldMap to render
    void SetWorldMap(const std::shared_ptr<WorldMap>& map);

    // Render all game elements
    void Render(sf::RenderWindow& window, const Camera& camera);

    // Shutdown and clean up resources
    void Shutdown();

private:
    bool isInitialized = false; // Flag to indicate if the renderer has been initialized.
    std::shared_ptr<WorldMap> worldMap; // Shared pointer to the WorldMap to be rendered.

    // Mutex for thread-safe rendering if needed.
    mutable std::mutex renderMutex;

    sf::Font font; // Font used for rendering text.

    // Hover functionality
    std::string hoveredAreaName; // Name of the area currently hovered over by the mouse.
    sf::Text hoveredAreaText; // SFML text object to display the hovered area's name.

    // Private helper functions to handle different rendering aspects.
    void renderWorldMap(sf::RenderWindow& window, const Camera& camera); // Renders the world map.
    void renderPlaceAreas(sf::RenderWindow& window, const Camera& camera); // Renders different areas in the world.
    void renderStations(sf::RenderWindow& window, const Camera& camera); // Renders stations on the map.
    void renderLines(sf::RenderWindow& window, const Camera& camera); // Renders transport lines on the map.
    void renderHoveredAreaName(sf::RenderWindow& window); // Renders the name of the currently hovered area.

    // Constants
    static constexpr float HOVER_OUTLINE_THICKNESS = 2.0f; // Thickness of the outline for hovered areas.

    // Line segments for collision detection
    std::vector<std::pair<sf::Vector2f, sf::Vector2f>> lineSegments;

    // Private helper functions
    bool rectangleIntersectsLine(const sf::FloatRect& rect, const sf::Vector2f& p1, const sf::Vector2f& p2);
    bool lineSegmentsIntersect(const sf::Vector2f& p1, const sf::Vector2f& p2,
        const sf::Vector2f& q1, const sf::Vector2f& q2);
};
