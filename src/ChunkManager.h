// src/ChunkManager.h
#pragma once

#include "Chunk.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include "FastNoiseLite.h"

class ChunkManager {
public:
    ChunkManager(int worldChunksX, int worldChunksY, int chunkSize, int tileSize);

    // Generate a chunk at specified coordinates
    Chunk generateChunk(int chunkX, int chunkY);

    // Access a chunk by its wrapped indices
    const Chunk& getChunk(int x, int y) const;

    void updateSettings(float newFrequency, int newSeed, float newLandThreshold);
    void regenerateWorld();

    // Total chunks in X and Y directions
    int WORLD_CHUNKS_X;
    int WORLD_CHUNKS_Y;

    float noiseFrequency;
    int noiseSeed;
    float landThreshold;

private:
    int CHUNK_SIZE;
    int TILE_SIZE;
    std::vector<Chunk> chunks;

    FastNoiseLite noise;

    float worldWidth;
    float worldHeight;
};