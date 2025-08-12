#include "Renderer.h"
#include "../Logger.h"
#include "../core/Components.h"
#include "../core/Constants.h"
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <entt/entt.hpp>
#include <iostream>

Renderer::Renderer()
    : _windowInstance(sf::VideoMode({Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT}),
                      Constants::WINDOW_TITLE),
      _clearColor(Constants::CLEAR_COLOR_R, Constants::CLEAR_COLOR_G, Constants::CLEAR_COLOR_B) {
    LOG_INFO("Renderer", "Renderer created and window initialized.");
    _windowInstance.setFramerateLimit(Constants::FRAMERATE_LIMIT);
}

Renderer::~Renderer() {
    LOG_INFO("Renderer", "Renderer destroyed.");
}

void Renderer::initialize() {
    LOG_INFO("Renderer", "Initializing Renderer.");
    LOG_INFO("Renderer", "Renderer initialized.");
}

TerrainRenderSystem &Renderer::getTerrainRenderSystem() {
    return _terrainRenderSystem;
}

void Renderer::renderFrame(const entt::registry &registry, const sf::View &view, sf::Time dt) {
    LOG_TRACE("Renderer", "Beginning render pass.");
    _windowInstance.setView(view);
    _windowInstance.clear(_clearColor);

    _terrainRenderSystem.render(registry, _windowInstance, view);
    LOG_TRACE("Renderer", "Terrain rendered.");
    _lineRenderSystem.render(registry, _windowInstance, view);
    LOG_TRACE("Renderer", "Lines rendered.");

    auto viewRegistry = registry.view<const PositionComponent, const RenderableComponent>();
    int entityCount = 0;
    for (auto entity : viewRegistry) {
        const auto &position = viewRegistry.get<const PositionComponent>(entity);
        const auto &renderable = viewRegistry.get<const RenderableComponent>(entity);

        sf::CircleShape shape(renderable.radius);
        shape.setFillColor(renderable.color);
        shape.setPosition(position.coordinates);
        
        shape.setOrigin({renderable.radius, renderable.radius});

        _windowInstance.draw(shape);
        entityCount++;
    }
    LOG_TRACE("Renderer", "Rendered %d entities.", entityCount);

    LOG_TRACE("Renderer", "Render pass complete.");
}

void Renderer::displayFrame() {
    _windowInstance.display();
}

void Renderer::cleanupResources() {
    LOG_INFO("Renderer", "Renderer cleanup initiated.");
    LOG_INFO("Renderer", "Renderer cleaned up.");
}

bool Renderer::isWindowOpen() const {
    return _windowInstance.isOpen();
}

sf::RenderWindow &Renderer::getWindowInstance() {
    return _windowInstance;
}

void Renderer::setClearColor(const sf::Color &color) {
    _clearColor = color;
    LOG_DEBUG("Renderer", "Clear color set to R:%d G:%d B:%d A:%d", color.r, color.g, color.b,
              color.a);
}

const sf::Color &Renderer::getClearColor() const {
    return _clearColor;
}

void Renderer::connectToEventBus(EventBus &eventBus) {
    m_windowCloseConnection =
        eventBus.sink<WindowCloseEvent>().connect<&Renderer::onWindowClose>(this);
}

void Renderer::onWindowClose(const WindowCloseEvent &event) {
    _windowInstance.close();
}
