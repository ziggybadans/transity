// src/systems/Renderer.h
#pragma once
#include <SFML/Graphics.hpp>  // Include SFML for graphical rendering
#include <vector>             // Include for storing a list of drawable entities
#include <memory>             // Include for managing shared pointers

// Abstract base class representing an entity that can be drawn on the screen
class DrawableEntity {
public:
    virtual void draw(sf::RenderWindow& window) = 0; // Pure virtual function to draw the entity
    virtual ~DrawableEntity() = default;  // Virtual destructor to ensure derived class destructors are called properly
};

class Renderer {
public:
    // Adds a drawable entity to the list of entities to be rendered
    void addDrawable(std::shared_ptr<DrawableEntity> drawable);

    // Renders all drawable entities in the list to the given window
    void render(sf::RenderWindow& window);

private:
    std::vector<std::shared_ptr<DrawableEntity>> drawables; // List of drawable entities to be rendered
};

// Summary:
// The Renderer class manages a collection of drawable entities and handles their rendering to the screen.
// It allows entities that implement the DrawableEntity interface to be added and drawn each frame.
// This design decouples the rendering logic from individual game objects, making rendering flexible and modular.