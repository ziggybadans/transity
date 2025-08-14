#pragma once

#include "ecs/ISystem.h"
#include "TerrainRenderSystem.h"
#include <entt/entt.hpp>

class ServiceLocator;

class TerrainMeshSystem : public ISystem, public IUpdatable {
public:
    explicit TerrainMeshSystem(ServiceLocator &serviceLocator);
    void update(sf::Time dt) override;

private:
    entt::registry &_registry;
    TerrainRenderSystem &_terrainRenderSystem;
};
