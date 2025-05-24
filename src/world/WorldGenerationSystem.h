#pragma once

#include <entt/entt.hpp>

#include "FastNoiseLite.h"
#include "../core/Components.h"

class WorldGenerationSystem {
public:
    WorldGenerationSystem(entt::registry& registry);

    void configureNoise ();

    void generateChunk(entt::entity chunkEntity);
    void generateWorld(int numChunksX, int numChunksY);

    int getSeed() const { return _seed; }
    float getFrequency() const { return _frequency; }
    FastNoiseLite::NoiseType getNoiseType() const { return _noiseType; }
    FastNoiseLite::FractalType getFractalType() const { return _fractalType; }
    int getOctaves() const { return _octaves; }
    float getLacunarity() const { return _lacunarity; }
    float getGain() const { return _gain; }

    void setSeed(int seed) { _seed = seed; }
    void setFrequency(float frequency) { _frequency = frequency; }
    void setNoiseType(FastNoiseLite::NoiseType noiseType) { _noiseType = noiseType; }
    void setFractalType(FastNoiseLite::FractalType fractalType) { _fractalType = fractalType; }
    void setOctaves(int octaves) { _octaves = octaves; }
    void setLacunarity(float lacunarity) { _lacunarity = lacunarity; }
    void setGain(float gain) { _gain = gain; }

private:
    entt::registry& _registry;

    FastNoiseLite _noiseGenerator;
    int _seed;
    float _frequency;
    FastNoiseLite::NoiseType _noiseType;
    FastNoiseLite::FractalType _fractalType;
    int _octaves;
    float _lacunarity;
    float _gain;
    
    const WorldGridComponent& getWorldGridSettings();
};