#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include "app/GameState.h"

struct PositionComponent;
struct RenderableComponent;

class CityRenderSystem {
public:
    CityRenderSystem();
    void render(entt::registry& registry, sf::RenderTarget& target, const GameState& gameState, const sf::Color& highlightColor);

private:
    static sf::Font loadFont();
    void renderCapital(sf::RenderTarget& target, const PositionComponent& position, const RenderableComponent& renderable, const GameState& gameState, const sf::Color& highlightColor);
    void renderTown(sf::RenderTarget& target, const PositionComponent& position, const RenderableComponent& renderable, const GameState& gameState, const sf::Color& highlightColor);
    void renderSuburb(sf::RenderTarget& target, const PositionComponent& position, const RenderableComponent& renderable, const GameState& gameState, const sf::Color& highlightColor);

    sf::Font m_font;
    sf::Text m_text;
};