#pragma once

#include "TerrainRenderSystem.h"
#include "ecs/ISystem.h"
#include "event/UIEvents.h" // Add this include
#include <entt/entt.hpp>

class ServiceLocator;
class WorldGenerationSystem;

class TerrainMeshSystem : public ISystem, public IUpdatable {
public:
    explicit TerrainMeshSystem(ServiceLocator &serviceLocator);
    void update(sf::Time dt) override;

private:
    void onThemeChanged(const ThemeChangedEvent &event); // Add this declaration

    entt::registry &_registry;
    TerrainRenderSystem &_terrainRenderSystem;
    const WorldGenerationSystem &_worldGenSystem;
    entt::scoped_connection _themeChangeConnection; // Add this member
};