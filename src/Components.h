#pragma once

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <string>

struct PositionComponent {
    sf::Vector2f coordinates;
};

struct RenderableComponent {
    sf::CircleShape shape;
};