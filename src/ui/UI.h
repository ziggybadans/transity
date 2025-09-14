#pragma once

#include "FastNoiseLite.h"
#include "app/GameState.h"
#include "event/EventBus.h"
#include "event/LineEvents.h"
#include "render/Camera.h"
#include "systems/world/WorldGenerationSystem.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

class TerrainRenderSystem;
class ServiceLocator;

class UI {
public:
    UI(sf::RenderWindow &window, TerrainRenderSystem &terrainRenderSystem,
       ServiceLocator &serviceLocator);
    ~UI();
    void initialize();
    void processEvent(const sf::Event &event);
    void update(sf::Time deltaTime, size_t numStationsInActiveLine);
    void renderFrame();
    void cleanupResources();

private:
    void drawPerformancePanel();
    void drawLoadingScreen();

    sf::RenderWindow &_window;
    ServiceLocator &_serviceLocator;
    TerrainRenderSystem &_terrainRenderSystem;

    bool _autoRegenerate;

    bool _visualizeChunkBorders = false;
    bool _visualizeCellBorders = false;
    bool _visualizeSuitabilityMap = false;
    int _selectedSuitabilityMap = 3;
    bool _isLodEnabled = false;
};