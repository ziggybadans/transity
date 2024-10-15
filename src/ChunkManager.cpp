// src/ChunkManager.cpp
#include "ChunkManager.h"
#include "Utilities.h"
#include <cmath>
#include <thread>
#include <vector>

ChunkManager::ChunkManager(int worldChunksX, int worldChunksY, int chunkSize, int tileSize)
    : WORLD_CHUNKS_X(worldChunksX), WORLD_CHUNKS_Y(worldChunksY),
    CHUNK_SIZE(chunkSize), TILE_SIZE(tileSize),
    landThreshold(0.5f), borderWidth(4.35f), attenuationFactor(0.243f), totalAmplitude(0.0f)
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

    for (const auto& layer : noiseLayers) {
        totalAmplitude += layer.amplitude;
    }

    // Pre-generate all chunks synchronously
    chunks.resize(WORLD_CHUNKS_X * WORLD_CHUNKS_Y);
    regenerateWorld();
}

// Function to regenerate the entire world
void ChunkManager::regenerateWorld() {
    int totalChunks = WORLD_CHUNKS_X * WORLD_CHUNKS_Y;
    chunks.resize(totalChunks);

    // Determine the number of hardware threads
    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4; // Fallback

    std::vector<std::thread> threads;
    int chunksPerThread = totalChunks / numThreads;

    for (unsigned int t = 0; t < numThreads; ++t) {
        int start = t * chunksPerThread;
        int end = (t == numThreads - 1) ? totalChunks : start + chunksPerThread;
        threads.emplace_back([this, start, end]() {
            for (int i = start; i < end; ++i) {
                int x = i % WORLD_CHUNKS_X;
                int y = i / WORLD_CHUNKS_X;
                chunks[i] = generateChunk(x, y);
            }
            });
    }

    for (auto& thread : threads) {
        thread.join();
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

    const float invWorldWidth = 1.0f / worldWidth;
    const float invWorldHeight = 1.0f / worldHeight;

    std::vector<float> tileHeights(CHUNK_SIZE * CHUNK_SIZE, 0.0f);
    std::vector<sf::Color> tileColors(CHUNK_SIZE * CHUNK_SIZE, sf::Color::Black);

    // Generate a chunk of tiles at the specified chunk coordinates
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            // Determine which LOD levels this tile should appear in
            // For simplicity:
            // - LOD0: every tile
            // - LOD1: every 2nd tile
            // - LOD2: every 4th tile

            // Calculate tile's world position
            float worldX = static_cast<float>((chunkX * CHUNK_SIZE + x) * TILE_SIZE);
            float worldY = static_cast<float>((chunkY * CHUNK_SIZE + y) * TILE_SIZE);

            float nx = worldX / (WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE);
            float ny = worldY / (WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE);

            float normalizedX = worldX * invWorldWidth;
            float normalizedY = worldY * invWorldHeight;

            // Calculate distance to the nearest edge (normalized between 0 and 1)
            float distanceToEdgeX = (normalizedX < 0.5f) ? normalizedX : (1.0f - normalizedX);
            float distanceToEdgeY = (normalizedY < 0.5f) ? normalizedY : (1.0f - normalizedY);
            float distanceToEdge = std::min(distanceToEdgeX, distanceToEdgeY) * borderWidth;

            // Introduce an attenuation factor that decreases as we approach the edges
            // The factor is squared to create a more gradual effect
            float edgeAttenuationFactor = std::pow(distanceToEdge, attenuationFactor);

            // Combine noise layers
            float height = 0.0f;
            for (const auto& layer : noiseLayers) {
                float noiseValue = layer.noise.GetNoise(worldX, worldY, 0.0f);
                noiseValue = (noiseValue + 1.0f) / 2.0f;
                height += noiseValue * layer.amplitude;
            }
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

            tileHeights[y * CHUNK_SIZE + x] = height;
            tileColors[y * CHUNK_SIZE + x] = color;

            // Define the four corners of the quad
            sf::Vertex topLeft(sf::Vector2f(worldX, worldY), color);
            sf::Vertex topRight(sf::Vector2f(worldX + tileSizeF, worldY), color);
            sf::Vertex bottomRight(sf::Vector2f(worldX + tileSizeF, worldY + tileSizeF), color);
            sf::Vertex bottomLeft(sf::Vector2f(worldX, worldY + tileSizeF), color);

            // Append the quad to the appropriate LOD vertex arrays
            chunk.verticesLOD0.append(topLeft);
            chunk.verticesLOD0.append(topRight);
            chunk.verticesLOD0.append(bottomRight);
            chunk.verticesLOD0.append(bottomLeft);

            /*
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
            */
        }
    }

    // Second pass: Aggregate LOD1 (2x2 tiles)
    for (int y = 0; y < CHUNK_SIZE; y += 2) {
        for (int x = 0; x < CHUNK_SIZE; x += 2) {
            sf::Color aggregatedColor(0, 0, 0);
            unsigned int tempR = 0, tempG = 0, tempB = 0, tempA = 0;
            int tilesAggregated = 0;

            for (int dy = 0; dy < 2; ++dy) {
                for (int dx = 0; dx < 2; ++dx) {
                    int currentX = x + dx;
                    int currentY = y + dy;

                    if (currentX < CHUNK_SIZE && currentY < CHUNK_SIZE) {
                        sf::Color tileColor = tileColors[currentY * CHUNK_SIZE + currentX];
                        tempR += tileColor.r;
                        tempG += tileColor.g;
                        tempB += tileColor.b;
                        tempA += tileColor.a;
                        tilesAggregated++;
                    }
                }
            }

            if (tilesAggregated > 0) {
                aggregatedColor.r = static_cast<sf::Uint8>(tempR / tilesAggregated);
                aggregatedColor.g = static_cast<sf::Uint8>(tempG / tilesAggregated);
                aggregatedColor.b = static_cast<sf::Uint8>(tempB / tilesAggregated);
                aggregatedColor.a = static_cast<sf::Uint8>(tempA / tilesAggregated);
            }

            float aggWorldX = static_cast<float>((chunkX * CHUNK_SIZE + x) * TILE_SIZE);
            float aggWorldY = static_cast<float>((chunkY * CHUNK_SIZE + y) * TILE_SIZE);

            sf::Vertex aggTopLeft(sf::Vector2f(aggWorldX, aggWorldY), aggregatedColor);
            sf::Vertex aggTopRight(sf::Vector2f(aggWorldX + 2 * tileSizeF, aggWorldY), aggregatedColor);
            sf::Vertex aggBottomRight(sf::Vector2f(aggWorldX + 2 * tileSizeF, aggWorldY + 2 * tileSizeF), aggregatedColor);
            sf::Vertex aggBottomLeft(sf::Vector2f(aggWorldX, aggWorldY + 2 * tileSizeF), aggregatedColor);

            chunk.verticesLOD1.append(aggTopLeft);
            chunk.verticesLOD1.append(aggTopRight);
            chunk.verticesLOD1.append(aggBottomRight);
            chunk.verticesLOD1.append(aggBottomLeft);
        }
    }

    // Third pass: Aggregate LOD2 (4x4 tiles)
    for (int y = 0; y < CHUNK_SIZE; y += 4) {
        for (int x = 0; x < CHUNK_SIZE; x += 4) {
            sf::Color aggregatedColor(0, 0, 0);
            unsigned int tempR = 0, tempG = 0, tempB = 0, tempA = 0;
            int tilesAggregated = 0;

            for (int dy = 0; dy < 4; ++dy) {
                for (int dx = 0; dx < 4; ++dx) {
                    int currentX = x + dx;
                    int currentY = y + dy;

                    if (currentX < CHUNK_SIZE && currentY < CHUNK_SIZE) {
                        sf::Color tileColor = tileColors[currentY * CHUNK_SIZE + currentX];
                        tempR += tileColor.r;
                        tempG += tileColor.g;
                        tempB += tileColor.b;
                        tempA += tileColor.a;
                        tilesAggregated++;
                    }
                }
            }

            if (tilesAggregated > 0) {
                aggregatedColor.r = static_cast<sf::Uint8>(tempR / tilesAggregated);
                aggregatedColor.g = static_cast<sf::Uint8>(tempG / tilesAggregated);
                aggregatedColor.b = static_cast<sf::Uint8>(tempB / tilesAggregated);
                aggregatedColor.a = static_cast<sf::Uint8>(tempA / tilesAggregated);
            }

            float aggWorldX = static_cast<float>((chunkX * CHUNK_SIZE + x) * TILE_SIZE);
            float aggWorldY = static_cast<float>((chunkY * CHUNK_SIZE + y) * TILE_SIZE);

            sf::Vertex aggTopLeft(sf::Vector2f(aggWorldX, aggWorldY), aggregatedColor);
            sf::Vertex aggTopRight(sf::Vector2f(aggWorldX + 4 * tileSizeF, aggWorldY), aggregatedColor);
            sf::Vertex aggBottomRight(sf::Vector2f(aggWorldX + 4 * tileSizeF, aggWorldY + 4 * tileSizeF), aggregatedColor);
            sf::Vertex aggBottomLeft(sf::Vector2f(aggWorldX, aggWorldY + 4 * tileSizeF), aggregatedColor);

            chunk.verticesLOD2.append(aggTopLeft);
            chunk.verticesLOD2.append(aggTopRight);
            chunk.verticesLOD2.append(aggBottomRight);
            chunk.verticesLOD2.append(aggBottomLeft);
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
