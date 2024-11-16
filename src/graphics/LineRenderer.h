#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "../world/WorldMap.h"

class LineRenderer {
public:
    LineRenderer();
    ~LineRenderer();

    bool Init();
    void SetWorldMap(const std::shared_ptr<WorldMap>& map);
    void Render(sf::RenderWindow& window, const Camera& camera);
    void Shutdown();

private:
    std::shared_ptr<WorldMap> worldMap;
    sf::Font font;
    sf::Text hoveredLineText;

    // Helper methods
    void renderLines(sf::RenderWindow& window, const Camera& camera);
};
