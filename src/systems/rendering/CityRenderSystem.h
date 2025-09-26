#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

struct PositionComponent;
struct RenderableComponent;

class CityRenderSystem {
public:
    CityRenderSystem();
    void render(entt::registry& registry, sf::RenderWindow& window, const sf::Color& highlightColor);

private:
    static sf::Font loadFont();
    void renderCapital(sf::RenderWindow& window, const PositionComponent& position, const RenderableComponent& renderable, const sf::Color& highlightColor);
    void renderTown(sf::RenderWindow& window, const PositionComponent& position, const RenderableComponent& renderable, const sf::Color& highlightColor);
    void renderSuburb(sf::RenderWindow& window, const PositionComponent& position, const RenderableComponent& renderable, const sf::Color& highlightColor);

    sf::Font m_font;
    sf::Text m_text;
};