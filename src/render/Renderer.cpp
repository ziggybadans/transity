#include "Renderer.h"
#include "Constants.h"
#include "Logger.h"
#include "components/RenderComponents.h"
#include "components/TrainComponents.h"
#include "systems/world/WorldGenerationSystem.h"
#include <entt/entt.hpp>

Renderer::Renderer(ColorManager &colorManager, sf::RenderWindow &window)
    : _colorManager(colorManager), _windowInstance(window),
      _clearColor(_colorManager.getBackgroundColor()),
      _renderTexture(),
      _terrainRenderSystem(colorManager), _lineRenderSystem(), _trainRenderSystem(),
      _pathRenderSystem(), _cityRenderSystem(), _lineEditingRenderSystem() {
    LOG_DEBUG("Renderer", "Renderer created and window initialized.");
    _windowInstance.setFramerateLimit(Constants::FRAMERATE_LIMIT);
}

Renderer::~Renderer() {
    LOG_DEBUG("Renderer", "Renderer destroyed.");
}

void Renderer::initialize() {
    LOG_INFO("Renderer", "Initializing Renderer.");
    sf::Vector2u windowSize = _windowInstance.getSize();
    sf::Vector2u textureSize = {static_cast<unsigned int>(windowSize.x * SSAA_FACTOR),
                                static_cast<unsigned int>(windowSize.y * SSAA_FACTOR)};

    sf::ContextSettings settings;
    unsigned int maxAntiAliasingLevel = sf::RenderTexture::getMaximumAntiAliasingLevel();
    settings.antiAliasingLevel = maxAntiAliasingLevel;
    LOG_INFO("Renderer", "Max supported anti-aliasing level: %u. Using: %u", maxAntiAliasingLevel, settings.antiAliasingLevel);

    if (!_renderTexture.resize({textureSize.x, textureSize.y}, settings)) {
        LOG_FATAL("Renderer", "Failed to create render texture.");
        throw std::runtime_error("Failed to create render texture.");
    }
    _renderTexture.setSmooth(true);

    _renderSprite = std::make_unique<sf::Sprite>(_renderTexture.getTexture());
    _renderSprite->setScale({1.0f / SSAA_FACTOR, 1.0f / SSAA_FACTOR});
    LOG_DEBUG("Renderer", "Render texture created with size: %d x %d", textureSize.x,
              textureSize.y);
}

void Renderer::clear() {
    _windowInstance.clear(_clearColor);
}

TerrainRenderSystem &Renderer::getTerrainRenderSystem() noexcept {
    return _terrainRenderSystem;
}

void Renderer::renderFrame(entt::registry &registry, GameState &gameState, const sf::View &view,
                           const WorldGenerationSystem &worldGen,
                           PassengerSpawnAnimationSystem &passengerSpawnAnimationSystem,
                           float interpolation) {
    sf::View ssaaView = view;
    ssaaView.setViewport({{0.f, 0.f}, {1.f, 1.f}});
    _renderTexture.setView(ssaaView);
    _renderTexture.clear(_clearColor);

    const sf::Color &highlightColor = _colorManager.getHighlightColor();

    _terrainRenderSystem.render(registry, _renderTexture, ssaaView, worldGen.getParams());

    if (gameState.currentInteractionMode == InteractionMode::CREATE_LINE
        || gameState.currentInteractionMode == InteractionMode::EDIT_LINE) {
        _cityRenderSystem.render(registry, _renderTexture, gameState, highlightColor);
        _lineRenderSystem.render(registry, _renderTexture, gameState, ssaaView, highlightColor);
    } else {
        _lineRenderSystem.render(registry, _renderTexture, gameState, ssaaView, highlightColor);
        _cityRenderSystem.render(registry, _renderTexture, gameState, highlightColor);
    }

    renderGenericEntities(_renderTexture, registry, highlightColor);

    _trainRenderSystem.render(registry, _renderTexture, highlightColor);
    _pathRenderSystem.render(registry, _renderTexture);
    passengerSpawnAnimationSystem.render(_renderTexture);
    _lineEditingRenderSystem.draw(_renderTexture, registry, gameState);

    _renderTexture.display();
    _renderTexture.setActive(false);
    if (_renderSprite) {
        _windowInstance.draw(*_renderSprite);
    }
}

void Renderer::renderGenericEntities(sf::RenderTarget &target, entt::registry &registry,
                                     const sf::Color &highlightColor) {
    auto viewRegistry = registry.view<const PositionComponent, const RenderableComponent>(
        entt::exclude<TrainTag, CityComponent>);

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
        const auto &position = viewRegistry.get<const PositionComponent>(entity);
        const auto &renderable = viewRegistry.get<const RenderableComponent>(entity);

        sf::CircleShape shape(renderable.radius.value);
        shape.setFillColor(renderable.color);
        shape.setOrigin({renderable.radius.value, renderable.radius.value});
        shape.setPosition(position.coordinates);
        target.draw(shape);

        if (registry.all_of<SelectedComponent>(entity)) {
            sf::CircleShape highlight(renderable.radius.value + 3.0f);
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(highlightColor);
            highlight.setOutlineThickness(2.0f);
            highlight.setOrigin({renderable.radius.value + 3.0f, renderable.radius.value + 3.0f});
            highlight.setPosition(position.coordinates);
            target.draw(highlight);
        }
    }
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