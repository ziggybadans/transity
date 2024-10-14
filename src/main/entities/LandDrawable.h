// src/entities/LandDrawable.h
#pragma once
#include "../systems/Renderer.h"
#include <SFML/Graphics.hpp>

class LandDrawable : public DrawableEntity {
public:
    LandDrawable(float x, float y, float height);
    void draw(sf::RenderWindow& window) override;

private:
    sf::RectangleShape shape;
};
