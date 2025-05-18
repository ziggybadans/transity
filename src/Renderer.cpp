#include "Renderer.h"
#include "Camera.h"
#include "Components.h"
#include "Logger.h"
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>

Renderer::Renderer(sf::RenderWindow& window)
    : m_window(window), m_clearColor(173, 216, 230) {
    LOG_INFO("Renderer", "Renderer created.");
    // std::cout << "Renderer created." << std::endl; // Original logging, can be removed
}

Renderer::~Renderer() {
    LOG_INFO("Renderer", "Renderer destroyed.");
    // std::cout << "Renderer destroyed." << std::endl; // Original logging, can be removed
}

void Renderer::init() {
    LOG_INFO("Renderer", "Initializing Renderer.");
    m_landShape.setSize({100, 100});
    m_landShape.setFillColor(sf::Color::White);
    m_landShape.setOrigin(m_landShape.getSize() / 2.0f);
    m_landShape.setPosition({50, 50});
    LOG_DEBUG("Renderer", "Land shape created at (%.1f, %.1f) with size (%.1f, %.1f).", m_landShape.getPosition().x, m_landShape.getPosition().y, m_landShape.getSize().x, m_landShape.getSize().y);
    // std::cout << "Renderer initialized. Land shape created." << std::endl; // Original logging
    LOG_INFO("Renderer", "Renderer initialized.");
}

void Renderer::render(entt::registry& registry, Camera& camera) {
    LOG_TRACE("Renderer", "Beginning render pass.");
    m_window.setView(camera.getView());
    m_window.clear(m_clearColor);

    m_window.draw(m_landShape);
    LOG_TRACE("Renderer", "Land shape drawn.");

    auto view = registry.view<PositionComponent, RenderableComponent>();
    int entityCount = 0;
    for (auto entity : view) {
        auto& position = view.get<PositionComponent>(entity);
        auto& renderable = view.get<RenderableComponent>(entity);

        renderable.shape.setPosition(position.coordinates);
        m_window.draw(renderable.shape);
        entityCount++;
    }
    LOG_TRACE("Renderer", "Rendered %d entities.", entityCount);
    LOG_TRACE("Renderer", "Render pass complete.");
}

void Renderer::cleanup() {
    LOG_INFO("Renderer", "Renderer cleanup initiated.");
    // std::cout << "Renderer cleaned up." << std::endl; // Original logging
    LOG_INFO("Renderer", "Renderer cleaned up.");
}

void Renderer::setClearColor(const sf::Color& color) {
    m_clearColor = color;
    LOG_DEBUG("Renderer", "Clear color set to R:%d G:%d B:%d A:%d", color.r, color.g, color.b, color.a);
}

const sf::Color& Renderer::getClearColor() const {
    return m_clearColor;
}

sf::Vector2f Renderer::getLandCenter() const {
    LOG_TRACE("Renderer", "Getting land center.");
    return m_landShape.getPosition();
}

sf::Vector2f Renderer::getLandSize() const {
    LOG_TRACE("Renderer", "Getting land size.");
    return m_landShape.getSize();
}