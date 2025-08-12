#pragma once

#include <entt/entt.hpp>
#include <vector>

#include "../core/Components.h"
#include "../event/EventBus.h"
#include "../event/InputEvents.h"
#include "FastNoiseLite.h"
#include "WorldData.h"

class WorldGenerationSystem {
public:
    WorldGenerationSystem(entt::registry &registry, EventBus &eventBus);
    ~WorldGenerationSystem();

    void configureNoise();

    void generateChunk(entt::registry &registry, entt::entity chunkEntity);
    void generateChunkData(ChunkComponent &chunk) const;
    sf::Vector2f getWorldSize();

    entt::registry &getRegistry() { return _registry; }

    const WorldGenParams &getParams() const { return _params; }
    WorldGenParams &getParams() { return _params; }
    void setParams(const WorldGenParams &params);

    void regenerate(const WorldGenParams &params);

private:
    entt::registry &_registry;
    EventBus &_eventBus;

    std::vector<FastNoiseLite> _noiseGenerators;  
    FastNoiseLite _coastlineDistortion;
    WorldGenParams _params;

    void generateContinentShape();

    
    entt::connection _regenerateWorldListener;

    const WorldGridComponent &getWorldGridSettings() const;
};
