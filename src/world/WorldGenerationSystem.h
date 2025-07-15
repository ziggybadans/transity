#pragma once

#include <entt/entt.hpp>

#include "FastNoiseLite.h"
#include "../core/Components.h"
#include "WorldData.h"
#include "../event/InputEvents.h"
#include "../event/EventBus.h"

class WorldGenerationSystem {
public:
    WorldGenerationSystem(entt::registry& registry, EventBus& eventBus);
    ~WorldGenerationSystem();

    void configureNoise ();

    void generateChunk(entt::entity chunkEntity);
    void generateWorld(int numChunksX, int numChunksY);
    sf::Vector2f getWorldSize();

    entt::registry& getRegistry() { return _registry; }

    const WorldGenParams& getParams() const { return _params; }
    void setParams(const WorldGenParams& params);

    void generateWorldFromComponent();

private:
    entt::registry& _registry;
    EventBus& _eventBus;

    FastNoiseLite _noiseGenerator;
    WorldGenParams _params;

    std::vector<sf::Vector2f> _islandShape;

    void onRegenerateWorldRequest(const RegenerateWorldRequestEvent& event);
    void onWorldGenParamsChange(const WorldGenParamsChangeEvent& event);

    // Event Listener Connections
    entt::connection _regenerateWorldListener; // <-- ENSURE THIS IS PRESENT
    entt::connection _worldGenParamsListener;  // <-- ENSURE THIS IS PRESENT
    
    const WorldGridComponent& getWorldGridSettings();
    std::vector<sf::Vector2f> generateIslandBaseShape();
    std::vector<sf::Vector2f> distortCoastline(const std::vector<sf::Vector2f>& baseShape);
};