// Station.h
#pragma once

#include <SFML/Graphics.hpp>

class Station {
public:
    Station(const sf::Vector2f& position, float baseRadius = 10.0f, float baseOutlineThickness = 2.0f);

    void Render(sf::RenderWindow& window, float zoomLevel) const;

    sf::Vector2f GetPosition() const;

private:
    sf::Vector2f position;
    mutable sf::CircleShape shape; // mutable to allow modification in const Render

    float baseRadius;
    float baseOutlineThickness;
};
