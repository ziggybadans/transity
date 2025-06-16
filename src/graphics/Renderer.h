#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <entt/entt.hpp>
#include "../world/TerrainRenderSystem.h"
#include "LineRenderSystem.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void initialize();
    void renderFrame(entt::registry& registry,
        const sf::View& view,
        sf::Time dt,
        bool visualizeNoise);
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
    LineRenderSystem _lineRenderSystem;
};