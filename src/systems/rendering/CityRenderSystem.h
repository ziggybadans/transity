#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

class CityRenderSystem {
public:
    CityRenderSystem();
    void render(entt::registry& registry, sf::RenderWindow& window, const sf::Color& highlightColor);

private:
    static sf::Font loadFont();

    sf::Font m_font;
    sf::Text m_text;
};