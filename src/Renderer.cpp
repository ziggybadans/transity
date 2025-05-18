#include "Renderer.h"
#include "Camera.h"
#include "Components.h"
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>

Renderer::Renderer(sf::RenderWindow& window)
    : m_window(window), m_clearColor(173, 216, 230) {
    std::cout << "Renderer created." << std::endl;
}

Renderer::~Renderer() {
    std::cout << "Renderer destroyed." << std::endl;
}

void Renderer::init() {
    m_landShape.setSize({100, 100});
    m_landShape.setFillColor(sf::Color::White);
    m_landShape.setOrigin(m_landShape.getSize() / 2.0f);
    m_landShape.setPosition({50, 50});

    std::cout << "Renderer initialized. Land shape created." << std::endl;
}

void Renderer::render(entt::registry& registry, Camera& camera) {
    m_window.setView(camera.getView());
    m_window.clear(m_clearColor);

    m_window.draw(m_landShape);

    auto view = registry.view<PositionComponent, RenderableComponent>();
    for (auto entity : view) {
        auto& position = view.get<PositionComponent>(entity);
        auto& renderable = view.get<RenderableComponent>(entity);

        renderable.shape.setPosition(position.coordinates);
        m_window.draw(renderable.shape);
    }

    m_window.display();
}

void Renderer::cleanup() {
    std::cout << "Renderer cleaned up." << std::endl;
}

void Renderer::setClearColor(const sf::Color& color) {
    m_clearColor = color;
}

const sf::Color& Renderer::getClearColor() const {
    return m_clearColor;
}

sf::Vector2f Renderer::getLandCenter() const {
    return m_landShape.getPosition();
}

sf::Vector2f Renderer::getLandSize() const {
    return m_landShape.getSize();
}