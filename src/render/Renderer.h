#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <entt/entt.hpp>

#include "LineRenderSystem.h"
#include "event/EventBus.h"
#include "event/InputEvents.h"
#include "systems/rendering/TerrainRenderSystem.h"
#include "render/ColorManager.h"
#include "render/TrainRenderSystem.h" // Add this include

class WorldGenerationSystem;

class Renderer {
public:
    Renderer(ColorManager &colorManager);
    ~Renderer();

    void initialize();
    void clear();
    void renderFrame(const entt::registry &registry, const sf::View &view,
                     const WorldGenerationSystem &worldGen, float interpolation);
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
    sf::RenderWindow _windowInstance;
    sf::Color _clearColor;
    TerrainRenderSystem _terrainRenderSystem;
    LineRenderSystem _lineRenderSystem;
    TrainRenderSystem _trainRenderSystem; // Add this member

    void onWindowClose(const WindowCloseEvent &event);
    void onThemeChanged(const ThemeChangedEvent &event);
    entt::scoped_connection m_windowCloseConnection;
    entt::scoped_connection m_themeChangedConnection;
};