#pragma once

#include "app/GameState.h"
#include "event/EventBus.h"
#include "event/InputEvents.h"
#include "render/ColorManager.h"
#include "systems/rendering/CityRenderSystem.h"
#include "systems/rendering/LineEditingRenderSystem.h"
#include "systems/rendering/LineRenderSystem.h"
#include "systems/rendering/PassengerSpawnAnimationSystem.h"
#include "systems/rendering/PathRenderSystem.h"
#include "systems/rendering/TerrainRenderSystem.h"
#include "systems/rendering/TrainRenderSystem.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <entt/entt.hpp>
#include <memory>

class WorldGenerationSystem;

class Renderer {
public:
    Renderer(ColorManager &colorManager, sf::RenderWindow &window);
    ~Renderer();

    void initialize();
    void clear();
    void renderFrame(entt::registry &registry, GameState &gameState, const sf::View &view,
                     const WorldGenerationSystem &worldGen,
                     PassengerSpawnAnimationSystem &passengerSpawnAnimationSystem,
                     float interpolation);
    void renderGenericEntities(sf::RenderTarget &target, entt::registry &registry,
                               const sf::Color &highlightColor);
    void displayFrame() noexcept;
    void cleanupResources() noexcept;
    bool isWindowOpen() const noexcept;
    sf::RenderWindow &getWindowInstance() noexcept;

    void setClearColor(const sf::Color &color) noexcept;
    const sf::Color &getClearColor() const noexcept;

    void connectToEventBus(EventBus &eventBus);
    TerrainRenderSystem &getTerrainRenderSystem() noexcept;

private:
    ColorManager &_colorManager;
    sf::RenderWindow &_windowInstance;
    sf::Color _clearColor;

    // For SSAA
    sf::RenderTexture _renderTexture;
    std::unique_ptr<sf::Sprite> _renderSprite;
    static constexpr float SSAA_FACTOR = 2.0f;

    TerrainRenderSystem _terrainRenderSystem;
    LineRenderSystem _lineRenderSystem;
    TrainRenderSystem _trainRenderSystem;
    PathRenderSystem _pathRenderSystem;
    CityRenderSystem _cityRenderSystem;
    LineEditingRenderSystem _lineEditingRenderSystem;

    void onWindowClose(const WindowCloseEvent &event);
    void onThemeChanged(const ThemeChangedEvent &event);
    entt::scoped_connection m_windowCloseConnection;
    entt::scoped_connection m_themeChangedConnection;
};