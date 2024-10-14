// src/ChunkManager.h
#pragma once

#include "Chunk.h"
#include <vector>
#include <SFML/Graphics.hpp>

class ChunkManager {
public:
    ChunkManager(int worldChunksX, int worldChunksY, int chunkSize, int tileSize);

    // Generate a chunk at specified coordinates
    Chunk generateChunk(int chunkX, int chunkY);

    // Access a chunk by its wrapped indices
    const Chunk& getChunk(int x, int y) const;

    // Total chunks in X and Y directions
    int WORLD_CHUNKS_X;
    int WORLD_CHUNKS_Y;

private:
    int CHUNK_SIZE;
    int TILE_SIZE;
    std::vector<Chunk> chunks;
};