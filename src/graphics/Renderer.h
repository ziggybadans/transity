#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "../world/WorldMap.h"
#include "Camera.h"
#include "../utility/ThreadPool.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Initialize the renderer
    bool Init(sf::RenderWindow& window, ThreadPool& threadPool);

    // Set the WorldMap to render
    void SetWorldMap(std::shared_ptr<WorldMap> map);

    // Render all game elements
    void Render(sf::RenderWindow& window, const Camera& camera);

    // Shutdown and clean up resources
    void Shutdown();

private:
    bool isInitialized;
    std::shared_ptr<WorldMap> worldMap;

    // Add other renderable components here (e.g., sprites, entities)
};