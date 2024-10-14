// src/entities/ContourDrawable.h
#pragma once
#include "../systems/Renderer.h"

class ContourDrawable : public DrawableEntity {
public:
    ContourDrawable(float x, float y, float height, float contourInterval);
    void draw(sf::RenderWindow& window) override;

private:
    sf::Vertex line[2];
    float height;
};