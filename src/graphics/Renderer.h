#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <entt/entt.hpp>
#include "../input/InteractionMode.h"
#include "../world/TerrainRenderSystem.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void initialize();
    void renderFrame(entt::registry& registry, const sf::View& view, sf::Time dt, InteractionMode currentMode);
    void displayFrame();
    void cleanupResources();
    bool isWindowOpen() const;
    sf::RenderWindow& getWindowInstance();

    void setClearColor(const sf::Color& color);
    const sf::Color& getClearColor() const;

private:
    sf::RenderWindow _windowInstance;
    sf::Color _clearColor;
    TerrainRenderSystem _terrainRenderSystem;
};