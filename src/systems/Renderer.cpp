// src/systems/Renderer.cpp
#include "Renderer.h"

// Adds a drawable entity to the list of entities to be rendered
void Renderer::addDrawable(std::shared_ptr<DrawableEntity> drawable) {
    drawables.push_back(drawable); // Store the drawable entity in the vector for later rendering
}

// Renders all drawable entities to the specified window
void Renderer::render(sf::RenderWindow& window) {
    for (const auto& drawable : drawables) { // Iterate through each drawable entity
        drawable->draw(window); // Draw the entity to the given window
    }
}

// Summary:
// The Renderer implementation maintains a list of drawable entities and provides methods to add these entities to
// the list and render them on the screen. The addDrawable method adds entities to be drawn, while the render method
// iterates over all added entities and invokes their draw function, ensuring all visuals are properly rendered each frame.