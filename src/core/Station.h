// Station.h
#pragma once

#include <SFML/Graphics.hpp>

class Station {
public:
    // Constructor that initializes the station at a given position with optional base radius and outline thickness.
    Station(const sf::Vector2f& position, float baseRadius = 10.0f, float baseOutlineThickness = 2.0f);

    // Renders the station to the provided window, taking zoom level and hover state into account.
    void Render(sf::RenderWindow& window, float zoomLevel, bool isHovered = false) const;

    // Returns the position of the station.
    sf::Vector2f GetPosition() const;

private:
    sf::Vector2f position; // Position of the station.
    mutable sf::CircleShape shape; // Shape representing the station, mutable to allow modification in const Render.

    float baseRadius; // Base radius of the station.
    float baseOutlineThickness; // Base thickness of the station outline.
};
