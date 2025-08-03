#include "Renderer.h"
#include "../core/Components.h"
#include "../Logger.h"
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include "../core/Constants.h"

Renderer::Renderer()
    : _windowInstance(sf::VideoMode({Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT}), Constants::WINDOW_TITLE)
    , _clearColor(Constants::CLEAR_COLOR_R, Constants::CLEAR_COLOR_G, Constants::CLEAR_COLOR_B) {
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

TerrainRenderSystem& Renderer::getTerrainRenderSystem() {
    return _terrainRenderSystem;
}

void Renderer::renderFrame(entt::registry& registry,
    const sf::View& view,
    sf::Time dt) {
    LOG_TRACE("Renderer", "Beginning render pass.");
    _windowInstance.setView(view);
    _windowInstance.clear(_clearColor);

    _terrainRenderSystem.render(registry, _windowInstance, view);
    LOG_TRACE("Renderer", "Terrain rendered.");
    _lineRenderSystem.render(registry, _windowInstance, view);
    LOG_TRACE("Renderer", "Lines rendered.");


    auto viewRegistry = registry.view<PositionComponent, RenderableComponent>();
    int entityCount = 0;
    for (auto entity : viewRegistry) {
        auto& position = viewRegistry.get<PositionComponent>(entity);
        auto& renderable = viewRegistry.get<RenderableComponent>(entity);

        renderable.shape.setPosition(position.coordinates);
        _windowInstance.draw(renderable.shape);
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

sf::RenderWindow& Renderer::getWindowInstance(){
    return _windowInstance;
}

void Renderer::setClearColor(const sf::Color& color) {
    _clearColor = color;
    LOG_DEBUG("Renderer", "Clear color set to R:%d G:%d B:%d A:%d", color.r, color.g, color.b, color.a);
}

const sf::Color& Renderer::getClearColor() const {
    return _clearColor;
}

void Renderer::connectToEventBus(EventBus& eventBus) {
    m_windowCloseConnection = eventBus.sink<WindowCloseEvent>().connect<&Renderer::onWindowClose>(this);
}

void Renderer::onWindowClose(const WindowCloseEvent& event) {
    _windowInstance.close();
}