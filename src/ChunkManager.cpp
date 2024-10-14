// src/ChunkManager.cpp
#include "ChunkManager.h"
#include "Utilities.h"
#include <cmath>

ChunkManager::ChunkManager(int worldChunksX, int worldChunksY, int chunkSize, int tileSize)
    : WORLD_CHUNKS_X(worldChunksX), WORLD_CHUNKS_Y(worldChunksY),
    CHUNK_SIZE(chunkSize), TILE_SIZE(tileSize)
{
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFrequency(0.001f);
    noise.SetSeed(1337);

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

            float nx = tilePosX / (WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE);
            float ny = tilePosY / (WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE);

            float height = noise.GetNoise(tilePosX, tilePosY, 0.0f);
            height = (height + 1.0f) / 2.0f;

            sf::Color color;
            if (height > 0.5f) {
                color = sf::Color(231, 232, 234);
                if (height > 0.8f) {
                    color = sf::Color(139, 69, 19);
                }
            }
            else {
                color = sf::Color(174, 223, 246);
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

            // Add contour lines at specific height thresholds
            float contourThreshold = 0.5f; // Example threshold
            if (std::abs(height - contourThreshold) < 0.02f) { // Adjust sensitivity
                // Draw horizontal line
                chunk.contourLines.append(sf::Vertex(sf::Vector2f(tilePosX, tilePosY + tileSizeF / 2), sf::Color::White));
                chunk.contourLines.append(sf::Vertex(sf::Vector2f(tilePosX + tileSizeF, tilePosY + tileSizeF / 2), sf::Color::White));

                // Draw vertical line
                chunk.contourLines.append(sf::Vertex(sf::Vector2f(tilePosX + tileSizeF / 2, tilePosY), sf::Color::White));
                chunk.contourLines.append(sf::Vertex(sf::Vector2f(tilePosX + tileSizeF / 2, tilePosY + tileSizeF), sf::Color::White));
            }
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
