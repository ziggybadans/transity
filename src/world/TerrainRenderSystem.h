#pragma once

#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>

#include "../core/Components.h"

class TerrainRenderSystem {
public:
    TerrainRenderSystem();

    void render(entt::registry& registry, sf::RenderTarget& target);
private:
    sf::RectangleShape _cellShape;

    const WorldGridComponent& getWorldGridSettings(entt::registry& registry);
};