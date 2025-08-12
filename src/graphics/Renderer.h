#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <entt/entt.hpp>

#include "../event/EventBus.h"
#include "../event/InputEvents.h"
#include "../world/TerrainRenderSystem.h"
#include "LineRenderSystem.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void initialize();
    void renderFrame(const entt::registry &registry, const sf::View &view, float interpolation);
    void displayFrame();
    void cleanupResources();
    bool isWindowOpen() const;
    sf::RenderWindow &getWindowInstance();

    void setClearColor(const sf::Color &color);
    const sf::Color &getClearColor() const;

    void connectToEventBus(EventBus &eventBus);
    TerrainRenderSystem &getTerrainRenderSystem();

private:
    sf::RenderWindow _windowInstance;
    sf::Color _clearColor;
    TerrainRenderSystem _terrainRenderSystem;
    LineRenderSystem _lineRenderSystem;

    void onWindowClose(const WindowCloseEvent &event);
    entt::connection m_windowCloseConnection;
};
