#include "UIManager.h"
#include "event/EventBus.h"
#include "systems/gameplay/CityPlacementSystem.h"

UIManager::UIManager(entt::registry &registry, EventBus &eventBus,
                     WorldGenerationSystem &worldGenerationSystem,
                     TerrainRenderSystem &terrainRenderSystem,
                     PerformanceMonitor &performanceMonitor, Camera &camera, GameState &gameState,
                     ColorManager &colorManager, sf::RenderWindow &window,
                     CityPlacementSystem &cityPlacementSystem)
    : _cityPlacementSystem(cityPlacementSystem) {
    _infoPanelUI = std::make_unique<InfoPanelUI>(registry, eventBus, gameState);
    _worldGenSettingsUI =
        std::make_unique<WorldGenSettingsUI>(eventBus, worldGenerationSystem, terrainRenderSystem);
    _debugUI = std::make_unique<DebugUI>(registry, performanceMonitor, camera, gameState,
                                         colorManager, eventBus, window);
    _interactionUI = std::make_unique<InteractionUI>(gameState, eventBus, window);
}

UIManager::~UIManager() = default;

void UIManager::draw(sf::Time deltaTime, size_t numStationsInActiveLine,
                     size_t numPointsInActiveLine) {
    _infoPanelUI->draw();
    _worldGenSettingsUI->draw();
    CityPlacementDebugInfo cityPlacementDebugInfo = _cityPlacementSystem.getDebugInfo();
    _debugUI->draw(deltaTime, cityPlacementDebugInfo);
    _interactionUI->draw(numStationsInActiveLine, numPointsInActiveLine);
}