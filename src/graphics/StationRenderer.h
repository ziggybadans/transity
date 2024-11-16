#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "../world/WorldMap.h"

class StationRenderer {
public:
    StationRenderer();
    ~StationRenderer();

    bool Init();
    void SetWorldMap(const std::shared_ptr<WorldMap>& map);
    void Render(sf::RenderWindow& window, const Camera& camera);
    void Shutdown();

private:
    std::shared_ptr<WorldMap> worldMap;
    sf::Font font;
    sf::Text stationNameText;

    // Helper methods
    void renderStations(sf::RenderWindow& window, const Camera& camera);
};
