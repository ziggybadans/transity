#pragma once

#include <entt/entt.hpp>

#include "FastNoiseLite.h"
#include "../core/Components.h"

class WorldGenerationSystem {
public:
    WorldGenerationSystem();

    void configureNoise (
        int seed, 
        float frequency, 
        FastNoiseLite::NoiseType noiseType,
        FastNoiseLite::FractalType fractalType,
        int octaves,
        float lacunarity,
        float gain);

    void generateChunk(entt::registry& registry, entt::entity chunkEntity);
    void generateWorld(entt::registry& registry, int numChunksX, int numChunksY);

private:
    FastNoiseLite _noiseGenerator;
    
    const WorldGridComponent& getWorldGridSettings(entt::registry& registry);
};