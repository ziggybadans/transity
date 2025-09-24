#pragma once

#include "ecs/ISystem.h"
#include "event/UIEvents.h"
#include "event/EventBus.h" 
#include <entt/entt.hpp>

class Renderer;
class WorldGenerationSystem;
class TerrainRenderSystem;

class TerrainMeshSystem : public ISystem, public IUpdatable {
public:
    explicit TerrainMeshSystem(entt::registry& registry, Renderer& renderer, const WorldGenerationSystem& worldGenSystem, EventBus& eventBus);
    void update(sf::Time dt) override;

private:
    void onThemeChanged(const ThemeChangedEvent &event); 

    entt::registry &_registry;
    TerrainRenderSystem &_terrainRenderSystem;
    const WorldGenerationSystem &_worldGenSystem;
    entt::scoped_connection _themeChangeConnection; 
};