// src/entities/ContourDrawable.cpp
#include "ContourDrawable.h"

ContourDrawable::ContourDrawable(float x, float y, float height, float contourInterval) : height(height) {
    if (static_cast<int>(height / contourInterval) != height / contourInterval) {
        // Not on a contour line
        line[0].position = sf::Vector2f(x, y);
        line[1].position = sf::Vector2f(x + 1.0f, y + 1.0f);
        line[0].color = sf::Color::Transparent;
        line[1].color = sf::Color::Transparent;
    }
    else {
        // Draw contour line
        line[0].position = sf::Vector2f(x, y);
        line[1].position = sf::Vector2f(x + 1.0f, y + 1.0f);
        line[0].color = sf::Color::White;
        line[1].color = sf::Color::White;
    }
}

void ContourDrawable::draw(sf::RenderWindow& window) {
    window.draw(line, 2, sf::Lines);
}
