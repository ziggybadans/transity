#include "UIManager.h"
#include "app/GameState.h"
#include "core/PerformanceMonitor.h"
#include "event/EventBus.h"
#include "render/Camera.h"
#include "render/ColorManager.h"
#include "systems/rendering/TerrainRenderSystem.h"
#include "systems/world/WorldGenerationSystem.h"
#include <SFML/Graphics/RenderWindow.hpp>

UIManager::UIManager(entt::registry &registry, EventBus &eventBus,
                     WorldGenerationSystem &worldGenerationSystem,
                     TerrainRenderSystem &terrainRenderSystem,
                     PerformanceMonitor &performanceMonitor, Camera &camera, GameState &gameState,
                     ColorManager &colorManager, sf::RenderWindow &window) {
    _infoPanelUI = std::make_unique<InfoPanelUI>(registry, eventBus, gameState);
    _worldGenSettingsUI =
        std::make_unique<WorldGenSettingsUI>(eventBus, worldGenerationSystem, terrainRenderSystem);
    _debugUI = std::make_unique<DebugUI>(performanceMonitor, camera, gameState, colorManager,
                                         eventBus, window);
    _interactionUI = std::make_unique<InteractionUI>(gameState, eventBus, window);
}

UIManager::~UIManager() = default;

void UIManager::draw(sf::Time deltaTime, size_t numStationsInActiveLine,
                     size_t numPointsInActiveLine) {
    _infoPanelUI->draw();
    _worldGenSettingsUI->draw();
    _debugUI->draw(deltaTime);
    _interactionUI->draw(numStationsInActiveLine, numPointsInActiveLine);
}