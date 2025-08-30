#pragma once

#include "TerrainRenderSystem.h"
#include "ecs/ISystem.h"
#include <entt/entt.hpp>

class ServiceLocator;
class WorldGenerationSystem;

class TerrainMeshSystem : public ISystem, public IUpdatable {
public:
    explicit TerrainMeshSystem(ServiceLocator &serviceLocator);
    void update(sf::Time dt) override;

private:
    entt::registry &_registry;
    TerrainRenderSystem &_terrainRenderSystem;
    const WorldGenerationSystem &_worldGenSystem;
};
