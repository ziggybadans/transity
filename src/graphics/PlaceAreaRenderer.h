#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "../world/WorldMap.h"

class PlaceAreaRenderer {
public:
    PlaceAreaRenderer();
    ~PlaceAreaRenderer();

    bool Init();
    void SetWorldMap(const std::shared_ptr<WorldMap>& map);
    void Render(sf::RenderWindow& window, const Camera& camera);
    void Shutdown();

private:
    std::shared_ptr<WorldMap> worldMap;

    // Helper methods
    void renderPlaceAreas(sf::RenderWindow& window, const Camera& camera);
};
