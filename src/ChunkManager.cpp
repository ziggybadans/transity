// src/ChunkManager.cpp
#include "ChunkManager.h"
#include "Utilities.h"
#include <cmath>

ChunkManager::ChunkManager(int worldChunksX, int worldChunksY, int chunkSize, int tileSize)
    : WORLD_CHUNKS_X(worldChunksX), WORLD_CHUNKS_Y(worldChunksY),
    CHUNK_SIZE(chunkSize), TILE_SIZE(tileSize),
    noiseFrequency(0.005f), noiseSeed(1337), landThreshold(0.5f)
{
    // Calculate world dimensions
    worldWidth = static_cast<float>(WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE);
    worldHeight = static_cast<float>(WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE);

    // Initialize noise settings
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFrequency(noiseFrequency); // Adjustable
    noise.SetSeed(noiseSeed);           // Adjustable

    // Pre-generate all chunks synchronously
    chunks.resize(WORLD_CHUNKS_X * WORLD_CHUNKS_Y);
    regenerateWorld();
}

// Function to regenerate the entire world
void ChunkManager::regenerateWorld() {
    for (int y = 0; y < WORLD_CHUNKS_Y; ++y) {
        for (int x = 0; x < WORLD_CHUNKS_X; ++x) {
            int index = y * WORLD_CHUNKS_X + x;
            chunks[index] = generateChunk(x, y);
        }
    }
}

// Update settings and regenerate world
void ChunkManager::updateSettings(float newFrequency, int newSeed, float newLandThreshold) {
    noiseFrequency = newFrequency;
    noiseSeed = newSeed;
    landThreshold = newLandThreshold;

    // Update noise generator with new settings
    noise.SetFrequency(noiseFrequency);
    noise.SetSeed(noiseSeed);

    // Regenerate all chunks with new settings
    regenerateWorld();
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
            float worldX = static_cast<float>((chunkX * CHUNK_SIZE + x) * TILE_SIZE);
            float worldY = static_cast<float>((chunkY * CHUNK_SIZE + y) * TILE_SIZE);

            float nx = worldX / (WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE);
            float ny = worldY / (WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE);

            float normalizedX = worldX / worldWidth;
            float normalizedY = worldY / worldHeight;

            float height = noise.GetNoise(worldX, worldY, 0.0f);
            height = (height + 1.0f) / 2.0f;

            sf::Color color;
            if (height > landThreshold) {
                color = sf::Color(231, 232, 234);
            }
            else {
                color = sf::Color(174, 223, 246);
            }

            // Define the four corners of the quad
            sf::Vertex topLeft(sf::Vector2f(worldX, worldY), color);
            sf::Vertex topRight(sf::Vector2f(worldX + tileSizeF, worldY), color);
            sf::Vertex bottomRight(sf::Vector2f(worldX + tileSizeF, worldY + tileSizeF), color);
            sf::Vertex bottomLeft(sf::Vector2f(worldX, worldY + tileSizeF), color);

            // Append the quad to the vertex array
            chunk.vertices.append(topLeft);
            chunk.vertices.append(topRight);
            chunk.vertices.append(bottomRight);
            chunk.vertices.append(bottomLeft);

            // Add contour lines at specific height thresholds
            float contourThreshold = 0.5f; // Example threshold
            if (std::abs(height - contourThreshold) < 0.02f) { // Adjust sensitivity
                // Draw horizontal line
                chunk.contourLines.append(sf::Vertex(sf::Vector2f(worldX, worldY + tileSizeF / 2), sf::Color::White));
                chunk.contourLines.append(sf::Vertex(sf::Vector2f(worldX + tileSizeF, worldY + tileSizeF / 2), sf::Color::White));

                // Draw vertical line
                chunk.contourLines.append(sf::Vertex(sf::Vector2f(worldX + tileSizeF / 2, worldY), sf::Color::White));
                chunk.contourLines.append(sf::Vertex(sf::Vector2f(worldX + tileSizeF / 2, worldY + tileSizeF), sf::Color::White));
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
