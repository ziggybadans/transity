// src/entities/LandDrawable.cpp
#include "LandDrawable.h"

LandDrawable::LandDrawable(float x, float y, float height) {
    shape.setPosition(x, y);
    shape.setSize(sf::Vector2f(1.0f, 1.0f)); // Adjust size as needed
    // Color based on height (e.g., higher land is darker)
    float colorValue = 128 + static_cast<int>(height * 127); // Normalize height to [0, 255]
    shape.setFillColor(sf::Color(colorValue, colorValue, colorValue));
}

void LandDrawable::draw(sf::RenderWindow& window) {
    window.draw(shape);
}
