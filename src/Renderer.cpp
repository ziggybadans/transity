#include "Renderer.h"
#include "Camera.h"       // For Camera class
#include "Components.h"   // For PositionComponent, RenderableComponent
#include <entt/entt.hpp> // For entt::registry
#include <SFML/Graphics.hpp>
#include <iostream>

Renderer::Renderer(sf::RenderWindow& window)
    : m_window(window), m_clearColor(173, 216, 230) { // Initialize with former oceanColor
    std::cout << "Renderer created." << std::endl;
}

Renderer::~Renderer() {
    std::cout << "Renderer destroyed." << std::endl;
}

void Renderer::init() {
    // Initialize land shape (moved from Game::Game())
    m_landShape.setSize({100, 100});
    m_landShape.setFillColor(sf::Color::White);
    m_landShape.setOrigin(m_landShape.getSize() / 2.0f);
    m_landShape.setPosition({50, 50}); // Initial position, game logic might update this if it's dynamic

    // m_clearColor is already set in the constructor, but could be adjusted here if needed.
    // For example, if it were configurable or loaded from a file.
    std::cout << "Renderer initialized. Land shape created." << std::endl;
}

void Renderer::render(entt::registry& registry, Camera& camera) {
    m_window.setView(camera.getView()); // Set the view from the camera
    m_window.clear(m_clearColor);       // Clear with the background color

    m_window.draw(m_landShape);         // Draw the land

    // Draw entities with PositionComponent and RenderableComponent
    auto view = registry.view<PositionComponent, RenderableComponent>();
    for (auto entity : view) {
        auto& position = view.get<PositionComponent>(entity);
        auto& renderable = view.get<RenderableComponent>(entity);

        renderable.shape.setPosition(position.coordinates);
        m_window.draw(renderable.shape);
    }

    // Potentially draw UI elements here, after resetting the view if necessary
    // m_window.setView(m_window.getDefaultView());
    // ... draw UI ...

    m_window.display(); // Display the rendered frame
}

void Renderer::cleanup() {
    // Clean up any resources allocated by the renderer
    // (e.g., shaders, textures not managed by SFML's resource classes directly)
    std::cout << "Renderer cleaned up." << std::endl;
}

void Renderer::setClearColor(const sf::Color& color) {
    m_clearColor = color;
}

const sf::Color& Renderer::getClearColor() const {
    return m_clearColor;
}