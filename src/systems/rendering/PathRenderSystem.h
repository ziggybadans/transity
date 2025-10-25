#pragma once

#include <entt/entt.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

class PathRenderSystem {
public:
    PathRenderSystem();
    void render(const entt::registry& registry, sf::RenderTarget& target);
};