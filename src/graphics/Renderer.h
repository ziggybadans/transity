#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <mutex>
#include "Camera.h"
#include "../utility/ThreadPool.h"
#include "../world/WorldMap.h"
#include "../world/CityManager.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Initialize the renderer
    bool Init(sf::RenderWindow& window, ThreadPool& threadPool);

    // Set the WorldMap to render
    void SetWorldMap(std::shared_ptr<WorldMap> map);

    // Set the CityManager
    void SetCityManager(CityManager* manager);

    // Set the City Circle Shape
    void SetCityCircleShape(std::shared_ptr<sf::CircleShape> shape);

    // Render all game elements
    void Render(sf::RenderWindow& window, const Camera& camera);

    // Shutdown and clean up resources
    void Shutdown();

private:
    bool isInitialized;
    std::shared_ptr<WorldMap> worldMap;

    // City rendering
    CityManager* cityManager;
    std::shared_ptr<sf::CircleShape> cityShape;

    // Font for city names
    std::shared_ptr<sf::Font> cityFont;

    // Base radius for city circles at default zoom level
    float baseCityRadius;

    // Mutex for thread-safe rendering if needed
    mutable std::mutex renderMutex;
};
