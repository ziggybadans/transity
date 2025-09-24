#include "TerrainMeshSystem.h"
#include "components/WorldComponents.h"
#include "render/Renderer.h"
#include "systems/world/WorldGenerationSystem.h"

TerrainMeshSystem::TerrainMeshSystem(entt::registry& registry, Renderer& renderer, const WorldGenerationSystem& worldGenSystem, EventBus& eventBus)
    : _registry(registry),
      _terrainRenderSystem(renderer.getTerrainRenderSystem()),
      _worldGenSystem(worldGenSystem) {
    _themeChangeConnection = eventBus.sink<ThemeChangedEvent>().connect<&TerrainMeshSystem::onThemeChanged>(this);
}

void TerrainMeshSystem::update(sf::Time dt) {
    _terrainRenderSystem.updateMeshes(_registry, _worldGenSystem.getParams());
}

void TerrainMeshSystem::onThemeChanged(const ThemeChangedEvent &event) {
    auto view = _registry.view<ChunkStateComponent>();
    for (auto entity : view) {
        auto &chunkState = view.get<ChunkStateComponent>(entity);
        chunkState.isMeshDirty = true;
    }
}