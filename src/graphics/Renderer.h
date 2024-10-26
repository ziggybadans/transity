#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <mutex>
#include "Camera.h"
#include "../utility/ThreadPool.h"
#include "../world/WorldMap.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Initialize the renderer
    bool Init(sf::RenderWindow& window, ThreadPool& threadPool);

    // Set the WorldMap to render
    void SetWorldMap(std::shared_ptr<WorldMap> map);

    // Set the City Circle Shape
    void SetCityCircleShape(std::shared_ptr<sf::CircleShape> shape);

    // Render all game elements
    void Render(sf::RenderWindow& window, const Camera& camera);

    // Shutdown and clean up resources
    void Shutdown();

private:
    bool isInitialized;
    std::shared_ptr<WorldMap> worldMap;

    // Mutex for thread-safe rendering if needed
    mutable std::mutex renderMutex;

    sf::Font font;
};
