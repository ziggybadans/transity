#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "../world/WorldMap.h"

class PlaceAreaRenderer {
public:
    PlaceAreaRenderer();
    ~PlaceAreaRenderer();

    /* Core Methods */
    bool Init();
    void Render(sf::RenderWindow& window, const Camera& camera);
    void Shutdown();

    /* Setters */
    void SetWorldMap(const std::shared_ptr<WorldMap>& map);

private:
    void RenderPlaceAreas(sf::RenderWindow& window, const Camera& camera);

    /* Renderer State */
    std::shared_ptr<WorldMap> m_worldMap;
};
