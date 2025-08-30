#include "TerrainMeshSystem.h"
#include "core/ServiceLocator.h"
#include "render/Renderer.h"
#include "systems/world/WorldGenerationSystem.h"

TerrainMeshSystem::TerrainMeshSystem(ServiceLocator &serviceLocator)
    : _registry(serviceLocator.registry),
      _terrainRenderSystem(serviceLocator.renderer.getTerrainRenderSystem()),
      _worldGenSystem(serviceLocator.worldGenerationSystem) {}

void TerrainMeshSystem::update(sf::Time dt) {
    _terrainRenderSystem.updateMeshes(_registry, _worldGenSystem.getParams());
}
