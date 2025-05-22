#pragma once

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <string>

struct PositionComponent {
    sf::Vector2f coordinates;
};

struct RenderableComponent {
    sf::CircleShape shape;
    int z_order;
};

struct StationComponent {
    std::vector<entt::entity> connectedLines;
};

struct LineComponent {
    sf::Color color;
    std::vector<entt::entity> stops;
};

struct ClickableComponent {
    float boundingRadius;
};

struct ActiveLineStationTag {
    int order = 0;
};