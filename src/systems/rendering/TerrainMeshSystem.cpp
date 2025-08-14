#include "TerrainMeshSystem.h"
#include "core/ServiceLocator.h"
#include "render/Renderer.h"

TerrainMeshSystem::TerrainMeshSystem(ServiceLocator &serviceLocator)
    : _registry(serviceLocator.registry),
      _terrainRenderSystem(serviceLocator.renderer.getTerrainRenderSystem()) {}

void TerrainMeshSystem::update(sf::Time dt) {
    _terrainRenderSystem.updateMeshes(_registry);
}
