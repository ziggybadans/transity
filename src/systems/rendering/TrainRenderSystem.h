#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <entt/entt.hpp>

class TrainRenderSystem {
public:
    TrainRenderSystem();
    void render(const entt::registry &registry, sf::RenderTarget &target, const sf::Color& highlightColor);

private:
    static sf::Font loadFont();

    sf::Font m_font;
    sf::Text m_text;
};