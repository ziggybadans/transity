#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entity/fwd.hpp>

class Camera;

class Renderer {
public:
    Renderer(sf::RenderWindow& window);
    ~Renderer();

    void init();
    void render(entt::registry& registry, Camera& camera);
    void cleanup();

    void setClearColor(const sf::Color& color);
    const sf::Color& getClearColor() const;

private:
    sf::RenderWindow& m_window;
    sf::Color m_clearColor;
    sf::RectangleShape m_landShape;
};