#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <entt/entt.hpp>

class TrainRenderSystem {
public:
    TrainRenderSystem();
    void render(const entt::registry &registry, sf::RenderWindow &window, const sf::Color& highlightColor);
};