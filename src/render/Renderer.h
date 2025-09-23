#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <entt/entt.hpp>

#include "event/EventBus.h"
#include "event/InputEvents.h"
#include "render/ColorManager.h"
#include "systems/rendering/LineRenderSystem.h"
#include "systems/rendering/PassengerSpawnAnimationSystem.h"
#include "systems/rendering/PathRenderSystem.h"
#include "systems/rendering/TerrainRenderSystem.h"
#include "systems/rendering/TrainRenderSystem.h"  // Include the train render system

class WorldGenerationSystem;

class Renderer {
public:
    Renderer(ColorManager &colorManager);
    ~Renderer();

    void initialize();
    void clear();
    void renderFrame(const entt::registry &registry, const sf::View &view,
                     const WorldGenerationSystem &worldGen,
                     PassengerSpawnAnimationSystem &passengerSpawnAnimationSystem,
                     float interpolation);
    void displayFrame() noexcept;
    void cleanupResources() noexcept;
    bool isWindowOpen() const noexcept;
    sf::RenderWindow &getWindowInstance() noexcept;

    void setClearColor(const sf::Color &color) noexcept;
    const sf::Color &getClearColor() const noexcept;

    void connectToEventBus(EventBus &eventBus);
    TerrainRenderSystem &getTerrainRenderSystem() noexcept;

private:
    static sf::Font loadFont();

    ColorManager &_colorManager;
    sf::RenderWindow _windowInstance;
    sf::Color _clearColor;
    TerrainRenderSystem _terrainRenderSystem;
    LineRenderSystem _lineRenderSystem;
    TrainRenderSystem _trainRenderSystem;
    PathRenderSystem _pathRenderSystem;
    sf::Font m_font;
    sf::Text m_text;

    void onWindowClose(const WindowCloseEvent &event);
    void onThemeChanged(const ThemeChangedEvent &event);
    entt::scoped_connection m_windowCloseConnection;
    entt::scoped_connection m_themeChangedConnection;
};