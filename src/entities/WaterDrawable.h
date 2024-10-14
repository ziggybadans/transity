// src/entities/WaterDrawable.h
#pragma once
#include "../systems/Renderer.h"
#include <SFML/Graphics.hpp>

class WaterDrawable : public DrawableEntity {
public:
    WaterDrawable(float x, float y);
    void draw(sf::RenderWindow& window) override;

private:
    sf::RectangleShape shape;
};
