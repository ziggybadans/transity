#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

class LineRenderSystem {
public:
    void render(const entt::registry &registry, sf::RenderWindow &window, const sf::View &view);
};