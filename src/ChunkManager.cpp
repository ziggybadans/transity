// src/ChunkManager.cpp
#include "ChunkManager.h"
#include "Utilities.h"
#include <cmath>

ChunkManager::ChunkManager(int worldChunksX, int worldChunksY, int chunkSize, int tileSize)
    : WORLD_CHUNKS_X(worldChunksX), WORLD_CHUNKS_Y(worldChunksY),
    CHUNK_SIZE(chunkSize), TILE_SIZE(tileSize)
{
    // Pre-generate all chunks synchronously
    chunks.resize(WORLD_CHUNKS_X * WORLD_CHUNKS_Y);
    for (int y = 0; y < WORLD_CHUNKS_Y; ++y) {
        for (int x = 0; x < WORLD_CHUNKS_X; ++x) {
            int index = y * WORLD_CHUNKS_X + x;
            chunks[index] = generateChunk(x, y);
        }
    }
}

Chunk ChunkManager::generateChunk(int chunkX, int chunkY) {
    chunkX = Utilities::wrapCoordinate(chunkX, WORLD_CHUNKS_X);
    chunkY = Utilities::wrapCoordinate(chunkY, WORLD_CHUNKS_Y);

    Chunk chunk;
    int totalTilesX = WORLD_CHUNKS_X * CHUNK_SIZE;
    int totalTilesY = WORLD_CHUNKS_Y * CHUNK_SIZE;

    // Calculate tile size in float for positioning
    float tileSizeF = static_cast<float>(TILE_SIZE);

    // Generate a chunk of tiles at the specified chunk coordinates
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            // Calculate tile's world position
            float tilePosX = static_cast<float>((chunkX * CHUNK_SIZE + x) * TILE_SIZE);
            float tilePosY = static_cast<float>((chunkY * CHUNK_SIZE + y) * TILE_SIZE);

            sf::Color color;
            if (chunkY == WORLD_CHUNKS_Y / 2) {
                color = sf::Color::Green;
            }
            else {
                // Alternate between land and water tiles to create a checkerboard pattern
                if (((chunkX * CHUNK_SIZE + x) + (chunkY * CHUNK_SIZE + y)) % 2 == 0) {
                    color = sf::Color::Green;
                }
                else {
                    color = sf::Color::Blue;
                }
            }

            // Define the four corners of the quad
            sf::Vertex topLeft(sf::Vector2f(tilePosX, tilePosY), color);
            sf::Vertex topRight(sf::Vector2f(tilePosX + tileSizeF, tilePosY), color);
            sf::Vertex bottomRight(sf::Vector2f(tilePosX + tileSizeF, tilePosY + tileSizeF), color);
            sf::Vertex bottomLeft(sf::Vector2f(tilePosX, tilePosY + tileSizeF), color);

            // Append the quad to the vertex array
            chunk.vertices.append(topLeft);
            chunk.vertices.append(topRight);
            chunk.vertices.append(bottomRight);
            chunk.vertices.append(bottomLeft);
        }
    }
    return chunk;
}

const Chunk& ChunkManager::getChunk(int x, int y) const {
    int wrappedX = Utilities::wrapCoordinate(x, WORLD_CHUNKS_X);
    int wrappedY = Utilities::wrapCoordinate(y, WORLD_CHUNKS_Y);
    int index = wrappedY * WORLD_CHUNKS_X + wrappedX;
    return chunks.at(index);
}
