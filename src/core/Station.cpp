// Station.cpp
#include "Station.h"

Station::Station(const sf::Vector2f& position, float baseRadius, float baseOutlineThickness)
    : position(position),
    baseRadius(baseRadius),
    baseOutlineThickness(baseOutlineThickness)
{
    shape.setRadius(baseRadius); // Base radius
    shape.setFillColor(sf::Color::White); // White fill
    shape.setOutlineColor(sf::Color::Black); // Black outline
    shape.setOutlineThickness(baseOutlineThickness); // Base outline thickness
    shape.setOrigin(shape.getRadius(), shape.getRadius()); // Center the origin
    shape.setPosition(position);
}

void Station::Render(sf::RenderWindow& window, float zoomLevel, bool isHovered) const {
    // Scale radius and outline thickness based on zoom level
    float scaledRadius = baseRadius * zoomLevel;
    float scaledOutlineThickness = baseOutlineThickness * zoomLevel;

    // Update shape's properties
    shape.setRadius(scaledRadius);
    shape.setOutlineThickness(scaledOutlineThickness);
    shape.setOrigin(scaledRadius, scaledRadius); // Re-center origin based on new radius
    shape.setFillColor(isHovered ? sf::Color::Blue : sf::Color::White); // Change color on hover

    window.draw(shape);
}

sf::Vector2f Station::GetPosition() const {
    return position;
}
