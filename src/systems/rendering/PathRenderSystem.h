#pragma once

#include <entt/entt.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

class PathRenderSystem {
public:
    PathRenderSystem();
    void render(const entt::registry& registry, sf::RenderWindow& window);
};