// src/ChunkManager.h
#pragma once

#include "Chunk.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include "FastNoiseLite.h"

// Structure to represent each noise layer
struct NoiseLayer {
    FastNoiseLite noise;
    float amplitude;
    float frequency;
    FastNoiseLite::NoiseType noiseType;
    int seed;

    // Properties specific to Cellular noise
    FastNoiseLite::CellularDistanceFunction cellularDistanceFunction;
    FastNoiseLite::CellularReturnType cellularReturnType;
    float cellularJitter;

    // Constructor for convenience
    NoiseLayer(FastNoiseLite::NoiseType type = FastNoiseLite::NoiseType_Perlin,
        float freq = 0.005f, float amp = 1.0f, int s = 1337)
        : amplitude(amp), frequency(freq), noiseType(type), seed(s),
        cellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean),
        cellularReturnType(FastNoiseLite::CellularReturnType_CellValue),
        cellularJitter(0.5f) // Default jitter
    {
        noise.SetNoiseType(noiseType);
        noise.SetFrequency(frequency);
        noise.SetSeed(seed);
        configureNoise();
    }

    // Method to apply specific configurations based on noise type
    void configureNoise() {
        if (noiseType == FastNoiseLite::NoiseType_Cellular) {
            noise.SetCellularDistanceFunction(cellularDistanceFunction);
            noise.SetCellularReturnType(cellularReturnType);
            noise.SetCellularJitter(cellularJitter);
        }
        // Add configurations for other noise types if necessary
    }
};

class ChunkManager {
public:
    ChunkManager(int worldChunksX, int worldChunksY, int chunkSize, int tileSize);

    // Generate a chunk at specified coordinates
    Chunk generateChunk(int chunkX, int chunkY);

    // Access a chunk by its wrapped indices
    const Chunk& getChunk(int x, int y) const;

    void regenerateWorld();

    // Total chunks in X and Y directions
    int WORLD_CHUNKS_X;
    int WORLD_CHUNKS_Y;

    float landThreshold;
    float borderWidth;
    float attenuationFactor;

    // Noise layers
    std::vector<NoiseLayer> noiseLayers;
private:
    int CHUNK_SIZE;
    int TILE_SIZE;
    std::vector<Chunk> chunks;

    FastNoiseLite noise;

    float worldWidth;
    float worldHeight;
};