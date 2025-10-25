#pragma once

#include "app/GameState.h"
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <vector>

class LineRenderSystem {
public:
    void render(const entt::registry &registry, sf::RenderTarget &target, const GameState& gameState, const sf::View &view, const sf::Color& highlightColor);

private:
    void renderFinalizedLines(const entt::registry &registry, sf::RenderTarget &target, const sf::Color& highlightColor);
    void renderActiveLinePreview(const entt::registry &registry, sf::RenderTarget &target);
    void renderSnappingIndicators(const entt::registry &registry, sf::RenderTarget &target);
};