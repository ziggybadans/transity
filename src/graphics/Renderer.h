#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <mutex>
#include "Camera.h"
#include "../world/WorldMap.h"

class PlaceAreaRenderer;

class Renderer {
public:
    Renderer();
    ~Renderer();

    /* Core Renderer Methods */
    bool Init(sf::RenderWindow& window);
    void Render(sf::RenderWindow& window, const Camera& camera);
    void Shutdown();

    /* Setters */
    void SetWorldMap(const std::shared_ptr<WorldMap>& map);

private:
    void RenderWorldMap(sf::RenderWindow& window, const Camera& camera);
    void RenderHoveredAreaName(sf::RenderWindow& window);

    /* Renderer State */
    bool m_isInitialized;
    std::shared_ptr<WorldMap> m_worldMap;
    std::mutex m_renderMutex;

    /* Specialized Renderers */
    std::unique_ptr<PlaceAreaRenderer> m_placeAreaRenderer;
};
