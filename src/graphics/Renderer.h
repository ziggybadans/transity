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
    bool isInitialized = false;
    std::shared_ptr<WorldMap> worldMap;

    // Mutex for thread-safe rendering if needed
    mutable std::mutex renderMutex;

    sf::Font font;

    // Hover functionality
    std::string hoveredAreaName;
    sf::Text hoveredAreaText;

    // Private helper functions
    void renderWorldMap(sf::RenderWindow& window, const Camera& camera);
    void renderPlaceAreas(sf::RenderWindow& window, const Camera& camera);
    void renderStations(sf::RenderWindow& window, const Camera& camera);
    void renderLines(sf::RenderWindow& window, const Camera& camera);
    void renderHoveredAreaName(sf::RenderWindow& window);

    // Constants
    static constexpr float HOVER_OUTLINE_THICKNESS = 2.0f;
};
