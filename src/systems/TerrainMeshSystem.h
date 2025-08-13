#pragma once

#include "../core/ISystem.h"
#include <entt/entt.hpp>
#include "../world/TerrainRenderSystem.h"

class ServiceLocator;

class TerrainMeshSystem : public ISystem {
public:
    explicit TerrainMeshSystem(ServiceLocator& serviceLocator);
    void update(sf::Time dt) override;

private:
    entt::registry& _registry;
    TerrainRenderSystem& _terrainRenderSystem;
};
