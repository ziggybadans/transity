#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <mutex>
#include "Camera.h"
#include "../world/WorldMap.h"

// Forward declarations for specialized renderers
class LineRenderer;
class StationRenderer;
class PlaceAreaRenderer;

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
    bool isInitialized;
    std::shared_ptr<WorldMap> worldMap;
    std::mutex renderMutex;

    // Specialized renderers
    std::unique_ptr<LineRenderer> lineRenderer;
    std::unique_ptr<StationRenderer> stationRenderer;
    std::unique_ptr<PlaceAreaRenderer> placeAreaRenderer;

    // Private helper functions
    void renderWorldMap(sf::RenderWindow& window, const Camera& camera);
    void renderHoveredAreaName(sf::RenderWindow& window);
};
