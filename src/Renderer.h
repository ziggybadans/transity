#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <entt/entt.hpp>
#include "InteractionMode.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init();
    void render(entt::registry& registry, const sf::View& view, sf::Time dt, InteractionMode currentMode);
    void display();
    void cleanup();
    bool isOpen() const;
    sf::RenderWindow& getWindow();

    void setClearColor(const sf::Color& color);
    const sf::Color& getClearColor() const;

    sf::Vector2f getLandCenter() const;
    sf::Vector2f getLandSize() const;

private:
    sf::RenderWindow m_windowInstance;
    sf::Color m_clearColor;
    sf::RectangleShape m_landShape;
};