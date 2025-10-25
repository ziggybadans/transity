#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class LineDrawer {
public:
    static void createThickLine(sf::VertexArray &vertices, const std::vector<sf::Vector2f> &points,
                                float thickness, sf::Color color);

    static void drawBarberPolePolyline(sf::RenderTarget &target,
                                       const std::vector<sf::Vector2f> &points, float thickness,
                                       const std::vector<sf::Color> &colors, float phaseOffset);
};