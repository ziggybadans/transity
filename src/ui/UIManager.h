#pragma once

#include "DebugUI.h"
#include "InfoPanelUI.h"
#include "InteractionUI.h"
#include "WorldGenSettingsUI.h"
#include "event/EventBus.h"
#include <SFML/System/Time.hpp>
#include <entt/entt.hpp>
#include <memory>

class WorldGenerationSystem;
class TerrainRenderSystem;
class PerformanceMonitor;
class Camera;
class GameState;
class ColorManager;
namespace sf {
class RenderWindow;
}

class UIManager {
public:
    UIManager(entt::registry &registry, EventBus &eventBus,
              WorldGenerationSystem &worldGenerationSystem,
              TerrainRenderSystem &terrainRenderSystem, PerformanceMonitor &performanceMonitor,
              Camera &camera, GameState &gameState, ColorManager &colorManager,
              sf::RenderWindow &window);
    ~UIManager();

    void draw(sf::Time deltaTime, size_t numStationsInActiveLine, size_t numPointsInActiveLine);

private:
    std::unique_ptr<InfoPanelUI> _infoPanelUI;
    std::unique_ptr<WorldGenSettingsUI> _worldGenSettingsUI;
    std::unique_ptr<DebugUI> _debugUI;
    std::unique_ptr<InteractionUI> _interactionUI;
};