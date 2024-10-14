// src/systems/Renderer.cpp
#include "Renderer.h"

void Renderer::addDrawable(std::shared_ptr<DrawableEntity> drawable) {
    drawables.push_back(drawable);
}

void Renderer::render(sf::RenderWindow& window) {
    for (const auto& drawable : drawables) {
        drawable->draw(window);
    }
}
