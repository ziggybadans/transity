#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class LineDrawer {
public:
    // Creates a thick, solid-colored line from a series of points.
    static void createThickLine(sf::VertexArray &vertices, const std::vector<sf::Vector2f> &points,
                                float thickness, sf::Color color);

    // Draws a multi-colored, "barber pole" style line, often used for shared segments.
    static void drawBarberPolePolyline(sf::RenderWindow &window,
                                       const std::vector<sf::Vector2f> &points, float thickness,
                                       const std::vector<sf::Color> &colors, float phaseOffset);
};