// src/systems/Renderer.h
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class DrawableEntity {
public:
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual ~DrawableEntity() = default;
};

class Renderer {
public:
    void addDrawable(std::shared_ptr<DrawableEntity> drawable);
    void render(sf::RenderWindow& window);

private:
    std::vector<std::shared_ptr<DrawableEntity>> drawables;
};
