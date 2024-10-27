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
    bool isInitialized = false;
    std::shared_ptr<WorldMap> worldMap;

    // Mutex for thread-safe rendering if needed
    mutable std::mutex renderMutex;

    sf::Font font;

    // Hover functionality
    std::string hoveredCityName;
    sf::Text hoveredCityText;

    // Private helper functions
    void renderWorldMap(sf::RenderWindow& window, const Camera& camera);
    void renderCities(sf::RenderWindow& window, const Camera& camera);
    void renderHoveredCityName(sf::RenderWindow& window);

    // City rendering helper
    void updateHoveredCity(const sf::Vector2f& mouseWorldPos, const City& city, float scaledCircleRadius, sf::CircleShape& circle);

    // Constants
    static constexpr float BASE_CIRCLE_RADIUS = 8.0f;
    static constexpr float MAX_CIRCLE_RADIUS = 6.0f;
};
