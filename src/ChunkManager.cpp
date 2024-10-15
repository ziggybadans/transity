// src/ChunkManager.cpp
#include "ChunkManager.h"
#include "Utilities.h"
#include <cmath>

ChunkManager::ChunkManager(int worldChunksX, int worldChunksY, int chunkSize, int tileSize)
    : WORLD_CHUNKS_X(worldChunksX), WORLD_CHUNKS_Y(worldChunksY),
    CHUNK_SIZE(chunkSize), TILE_SIZE(tileSize),
    landThreshold(0.5f), borderWidth(4.35f), attenuationFactor(0.243f)
{
    // Calculate world dimensions
    worldWidth = static_cast<float>(WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE);
    worldHeight = static_cast<float>(WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE);

    // Initialize multiple noise layers
    // Layer 1: Base terrain using Perlin noise
    NoiseLayer baseLayer(FastNoiseLite::NoiseType_Perlin, 0.0075f, 0.6f, 1337);
    noiseLayers.push_back(baseLayer);

    // Layer 2: Elevation details using Cellular noise
    NoiseLayer cellularLayer(FastNoiseLite::NoiseType_OpenSimplex2, 0.001f, 1.0f, 42);
    // Customize cellular-specific properties
    cellularLayer.cellularDistanceFunction = FastNoiseLite::CellularDistanceFunction_EuclideanSq;
    cellularLayer.cellularReturnType = FastNoiseLite::CellularReturnType_Distance2;
    cellularLayer.cellularJitter = 0.8f;
    cellularLayer.configureNoise();
    noiseLayers.push_back(cellularLayer);

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

Chunk ChunkManager::generateChunk(int chunkX, int chunkY) {
    chunkX = Utilities::wrapCoordinate(chunkX, WORLD_CHUNKS_X);
    chunkY = Utilities::wrapCoordinate(chunkY, WORLD_CHUNKS_Y);

    Chunk chunk;
    int totalTilesX = WORLD_CHUNKS_X * CHUNK_SIZE;
    int totalTilesY = WORLD_CHUNKS_Y * CHUNK_SIZE;

    // Calculate tile size in float for positioning
    float tileSizeF = static_cast<float>(TILE_SIZE);

    // Precompute total amplitude for normalization
    float totalAmplitude = 0.0f;
    for (const auto& layer : noiseLayers) {
        totalAmplitude += layer.amplitude;
    }

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

            // Calculate distance to the nearest edge (normalized between 0 and 1)
            float distanceToEdgeX = std::min(normalizedX, 1.0f - normalizedX);
            float distanceToEdgeY = std::min(normalizedY, 1.0f - normalizedY);
            float distanceToEdge = std::min(distanceToEdgeX, distanceToEdgeY) * borderWidth;

            // Introduce an attenuation factor that decreases as we approach the edges
            // The factor is squared to create a more gradual effect
            float edgeAttenuationFactor = std::pow(distanceToEdge, attenuationFactor);

            // Combine noise layers
            float height = 0.0f;
            for (const auto& layer : noiseLayers) {
                float noiseValue = layer.noise.GetNoise(worldX, worldY, 0.0f);
                // Normalize noise value from [-1, 1] to [0, 1]
                noiseValue = (noiseValue + 1.0f) / 2.0f;
                height += noiseValue * layer.amplitude;
            }

            // Normalize the combined height
            height /= totalAmplitude;

            // Apply the attenuation factor to reduce height near edges
            height *= edgeAttenuationFactor;

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
