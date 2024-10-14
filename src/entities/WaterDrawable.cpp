// src/entities/WaterDrawable.cpp
#include "WaterDrawable.h"

WaterDrawable::WaterDrawable(float x, float y) {
    shape.setPosition(x, y);
    shape.setSize(sf::Vector2f(1.0f, 1.0f)); // Adjust size as needed
    shape.setFillColor(sf::Color::Blue); // Water color
}

void WaterDrawable::draw(sf::RenderWindow& window) {
    window.draw(shape);
}
