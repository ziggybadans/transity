#include "TerrainMeshSystem.h"
#include "components/WorldComponents.h" // Add this include
#include "core/ServiceLocator.h"
#include "event/EventBus.h" // Add this include
#include "render/Renderer.h"
#include "systems/world/WorldGenerationSystem.h"

TerrainMeshSystem::TerrainMeshSystem(ServiceLocator &serviceLocator)
    : _registry(serviceLocator.registry),
      _terrainRenderSystem(serviceLocator.renderer.getTerrainRenderSystem()),
      _worldGenSystem(serviceLocator.worldGenerationSystem) {
    // Add these lines to subscribe to the event
    _themeChangeConnection = serviceLocator.eventBus.sink<ThemeChangedEvent>().connect<&TerrainMeshSystem::onThemeChanged>(this);
}

void TerrainMeshSystem::update(sf::Time dt) {
    _terrainRenderSystem.updateMeshes(_registry, _worldGenSystem.getParams());
}

// Add this entire function
void TerrainMeshSystem::onThemeChanged(const ThemeChangedEvent &event) {
    auto view = _registry.view<ChunkStateComponent>();
    for (auto entity : view) {
        auto &chunkState = view.get<ChunkStateComponent>(entity);
        chunkState.isMeshDirty = true;
    }
}