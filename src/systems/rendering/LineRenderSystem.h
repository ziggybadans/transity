#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <vector>

#include "app/GameState.h" // Add this include

class LineRenderSystem {
public:
    // Update signature
    void render(const entt::registry &registry, sf::RenderWindow &window, const GameState& gameState, const sf::View &view, const sf::Color& highlightColor);
};