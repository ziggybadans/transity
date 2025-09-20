#include "Renderer.h"
#include "Constants.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "components/RenderComponents.h"
#include "systems/world/WorldGenerationSystem.h"
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <entt/entt.hpp>
#include <iostream>
#include <stdexcept>

sf::Font Renderer::loadFont() {
    sf::Font font;
    if (!font.openFromFile("data/fonts/font.TTF")) {
        const std::string errorMsg = "Failed to load font: data/fonts/font.TTF";
        LOG_ERROR("Renderer", errorMsg.c_str());
        throw std::runtime_error(errorMsg);
    }
    return font;
}

Renderer::Renderer(ColorManager &colorManager)
    : _colorManager(colorManager),
      _windowInstance(sf::VideoMode({Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT}),
                      Constants::WINDOW_TITLE,
                      sf::Style::Default,
                      sf::State::Windowed,
                      sf::ContextSettings{0u, 0u, 8u}),
      _clearColor(_colorManager.getBackgroundColor()), 
      _terrainRenderSystem(colorManager),
      _lineRenderSystem(),
      _trainRenderSystem(),
      _pathRenderSystem(),
      m_font(loadFont()),
      m_text(m_font)
 {
    LOG_DEBUG("Renderer", "Renderer created and window initialized.");
    _windowInstance.setFramerateLimit(Constants::FRAMERATE_LIMIT);
    m_text.setCharacterSize(24);
    m_text.setFillColor(sf::Color::Black);
}

Renderer::~Renderer() {
    LOG_DEBUG("Renderer", "Renderer destroyed.");
}

void Renderer::initialize() {
    LOG_INFO("Renderer", "Initializing Renderer.");
    LOG_INFO("Renderer", "Renderer initialized.");
}

void Renderer::clear() {
    _windowInstance.clear(_clearColor);
}

TerrainRenderSystem &Renderer::getTerrainRenderSystem() noexcept {
    return _terrainRenderSystem;
}

void Renderer::renderFrame(const entt::registry &registry, const sf::View &view,
                           const WorldGenerationSystem &worldGen, float interpolation) {
    _windowInstance.setView(view);
    _windowInstance.clear(_clearColor);

    const sf::Color& landColor = _colorManager.getLandColor();
    sf::Color highlightColor(255 - landColor.r, 255 - landColor.g, 255 - landColor.b);

    _terrainRenderSystem.render(registry, _windowInstance, view, worldGen.getParams());
    _lineRenderSystem.render(registry, _windowInstance, view, highlightColor);

    auto viewRegistry = registry.view<const PositionComponent, const RenderableComponent>();

    std::vector<entt::entity> sortedEntities;
    for (auto entity : viewRegistry) {
        sortedEntities.push_back(entity);
    }

    std::sort(sortedEntities.begin(), sortedEntities.end(), [&](const auto &a, const auto &b) {
        const auto &renderableA = viewRegistry.get<const RenderableComponent>(a);
        const auto &renderableB = viewRegistry.get<const RenderableComponent>(b);
        return renderableA.zOrder.value < renderableB.zOrder.value;
    });

    for (auto entity : sortedEntities) {
        // Skip rendering trains, as they are handled by TrainRenderSystem
        if (registry.all_of<TrainComponent>(entity)) {
            continue;
        }

        const auto &position = viewRegistry.get<const PositionComponent>(entity);
        const auto &renderable = viewRegistry.get<const RenderableComponent>(entity);

        sf::CircleShape shape(renderable.radius.value);
        shape.setFillColor(renderable.color);
        shape.setPosition(position.coordinates);

        shape.setOrigin({renderable.radius.value, renderable.radius.value});

        _windowInstance.draw(shape);

        // Draw passenger count for cities
        if (auto* city = registry.try_get<const CityComponent>(entity)) {
            if (!city->waitingPassengers.empty()) {
                m_text.setString(std::to_string(city->waitingPassengers.size()));
                sf::FloatRect textBounds = m_text.getLocalBounds();
                m_text.setOrigin({textBounds.position.x + textBounds.size.x / 2.0f,
                                  textBounds.position.y + textBounds.size.y / 2.0f});
                m_text.setPosition({position.coordinates.x + renderable.radius.value + 10.0f, position.coordinates.y});
                _windowInstance.draw(m_text);
            }
        }

        // Draw highlight if selected
        if (registry.all_of<SelectedComponent>(entity)) {
            sf::CircleShape highlight(renderable.radius.value + 3.0f);
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(highlightColor);
            highlight.setOutlineThickness(2.0f);
            highlight.setOrigin({renderable.radius.value + 3.0f, renderable.radius.value + 3.0f});
            highlight.setPosition(position.coordinates);
            _windowInstance.draw(highlight);
        }
    }

        _trainRenderSystem.render(registry, _windowInstance, highlightColor); // Call the train render system
        _pathRenderSystem.render(registry, _windowInstance); // Call the path render system
}

void Renderer::displayFrame() noexcept {
    _windowInstance.display();
}

void Renderer::cleanupResources() noexcept {
    LOG_INFO("Renderer", "Renderer cleanup initiated.");
    LOG_INFO("Renderer", "Renderer cleaned up.");
}

bool Renderer::isWindowOpen() const noexcept {
    return _windowInstance.isOpen();
}

sf::RenderWindow &Renderer::getWindowInstance() noexcept {
    return _windowInstance;
}

void Renderer::setClearColor(const sf::Color &color) noexcept {
    _clearColor = color;
    LOG_DEBUG("Renderer", "Clear color set to R:%d G:%d B:%d A:%d", color.r, color.g, color.b,
              color.a);
}

const sf::Color &Renderer::getClearColor() const noexcept {
    return _clearColor;
}

void Renderer::connectToEventBus(EventBus &eventBus) {
    m_windowCloseConnection =
        eventBus.sink<WindowCloseEvent>().connect<&Renderer::onWindowClose>(this);
    m_themeChangedConnection =
        eventBus.sink<ThemeChangedEvent>().connect<&Renderer::onThemeChanged>(this);
}

void Renderer::onWindowClose(const WindowCloseEvent &event) {
    _windowInstance.close();
}

void Renderer::onThemeChanged(const ThemeChangedEvent &event) {
    setClearColor(_colorManager.getBackgroundColor());
}