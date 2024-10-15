// src/ChunkManager.cpp
#include "ChunkManager.h"
#include "Utilities.h"

#include <cmath>
#include <thread>
#include <vector>
#include <future>
#include <iostream>

constexpr float DEFAULT_LAND_THRESHOLD = 0.5f;
constexpr float DEFAULT_BORDER_WIDTH = 4.35f;
constexpr float DEFAULT_ATTENUATION_FACTOR = 0.243f;

ChunkManager::ChunkManager(int worldChunksX, int worldChunksY, int chunkSize, int tileSize)
    : WORLD_CHUNKS_X(worldChunksX), WORLD_CHUNKS_Y(worldChunksY),
    CHUNK_SIZE(chunkSize), TILE_SIZE(tileSize),
    landThreshold(DEFAULT_LAND_THRESHOLD), borderWidth(DEFAULT_BORDER_WIDTH), attenuationFactor(DEFAULT_ATTENUATION_FACTOR), totalAmplitude(0.0f),
    heightMap(nullptr)
{
    initializeNoiseLayers();
    initializeWorldDimensions();
    regenerateWorld();
}

void ChunkManager::setHeightMap(HeightMap* hm) {
    heightMap = hm;
    if (heightMap) {
        useRealHeightMap = true;
    }
    else {
        useRealHeightMap = false;
    }
    regenerateWorld(); // Regenerate world with the new heightmap
}

// Function to regenerate the entire world
void ChunkManager::regenerateWorld() {
    int totalChunks = WORLD_CHUNKS_X * WORLD_CHUNKS_Y;
    chunks.resize(totalChunks);

    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;

    std::vector<std::future<void>> futures;
    int chunksPerThread = totalChunks / numThreads;

    for (unsigned int t = 0; t < numThreads; ++t) {
        int start = t * chunksPerThread;
        int end = (t == numThreads - 1) ? totalChunks : start + chunksPerThread;

        futures.emplace_back(std::async(std::launch::async, [this, start, end]() {
            for (int i = start; i < end; ++i) {
                int x = i % WORLD_CHUNKS_X;
                int y = i / WORLD_CHUNKS_X;
                chunks[i] = generateChunk(x, y);
            }
            }));
    }

    for (auto& future : futures) {
        future.get();
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
            // Calculate tile's world position
            float worldX = static_cast<float>((chunkX * CHUNK_SIZE + x) * TILE_SIZE);
            float worldY = static_cast<float>((chunkY * CHUNK_SIZE + y) * TILE_SIZE);

            // Calculate tile coordinates
            int tileX = chunkX * CHUNK_SIZE + x;
            int tileY = chunkY * CHUNK_SIZE + y;

            float height = 0.0f;
            if (useRealHeightMap && heightMap) {
                float invScaleX = static_cast<float>(heightMap->getWidth()) / totalTilesX;
                float invScaleY = static_cast<float>(heightMap->getHeight()) / totalTilesY;

                // Use tile coordinates instead of world coordinates for heightmap
                height = heightMap->getScaledHeight(static_cast<float>(tileX), static_cast<float>(tileY), invScaleX, invScaleY);
                landThreshold = 0.0f;
            }
            else {

                float nx = worldX / (WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE);
                float ny = worldY / (WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE);

                float normalizedX = worldX * invWorldWidth;
                float normalizedY = worldY * invWorldHeight;

                float edgeDistance = calculateEdgeDistance(normalizedX, normalizedY);
                float edgeAttenuationFactor = std::pow(edgeDistance, attenuationFactor);

                // Combine noise layers
                height = 0.0f;
                for (const auto& layer : noiseLayers) {
                    float noiseValue = layer.noise.GetNoise(worldX, worldY, 0.0f);
                    noiseValue = (noiseValue + 1.0f) / 2.0f;
                    height += noiseValue * layer.amplitude;
                }
                height /= totalAmplitude;

                // Apply the attenuation factor to reduce height near edges
                height *= edgeAttenuationFactor;
            }

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
            sf::Color aggregatedColor = aggregateTiles(tileColors, x, y, 2);

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
            sf::Color aggregatedColor = aggregateTiles(tileColors, x, y, 4);

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

sf::Color ChunkManager::aggregateTiles(const std::vector<sf::Color>& tileColors,
    int x, int y, int stepSize) const {
    unsigned int tempR = 0, tempG = 0, tempB = 0, tempA = 0;
    int tilesAggregated = 0;

    for (int dy = 0; dy < stepSize; ++dy) {
        for (int dx = 0; dx < stepSize; ++dx) {
            int currentX = x + dx;
            int currentY = y + dy;

            if (currentX < CHUNK_SIZE && currentY < CHUNK_SIZE) {
                sf::Color color = tileColors[currentY * CHUNK_SIZE + currentX];
                tempR += color.r;
                tempG += color.g;
                tempB += color.b;
                tempA += color.a;
                tilesAggregated++;
            }
        }
    }

    sf::Color aggregatedColor(0, 0, 0);
    if (tilesAggregated > 0) {
        aggregatedColor.r = static_cast<sf::Uint8>(tempR / tilesAggregated);
        aggregatedColor.g = static_cast<sf::Uint8>(tempG / tilesAggregated);
        aggregatedColor.b = static_cast<sf::Uint8>(tempB / tilesAggregated);
        aggregatedColor.a = static_cast<sf::Uint8>(tempA / tilesAggregated);
    }
    return aggregatedColor;
}

void ChunkManager::initializeNoiseLayers() {
    noiseLayers.emplace_back(FastNoiseLite::NoiseType_Perlin, 0.0075f, 0.6f, 1337);

    NoiseLayer cellularLayer(FastNoiseLite::NoiseType_OpenSimplex2, 0.001f, 1.0f, 42);
    cellularLayer.cellularDistanceFunction = FastNoiseLite::CellularDistanceFunction_EuclideanSq;
    cellularLayer.cellularReturnType = FastNoiseLite::CellularReturnType_Distance2;
    cellularLayer.cellularJitter = 0.8f;
    cellularLayer.configureNoise();
    noiseLayers.push_back(cellularLayer);

    for (const auto& layer : noiseLayers) {
        totalAmplitude += layer.amplitude;
    }
}

void ChunkManager::initializeWorldDimensions() {
    if (useRealHeightMap && heightMap) {
        worldWidth = static_cast<float>(WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE);
        worldHeight = static_cast<float>(WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE);
    }
    else {
        // Procedural generation
        worldWidth = static_cast<float>(WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE);
        worldHeight = static_cast<float>(WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE);
    }

    // Pre-generate all chunks synchronously
    chunks.resize(WORLD_CHUNKS_X * WORLD_CHUNKS_Y);
}

float ChunkManager::calculateEdgeDistance(float normalizedX, float normalizedY) const {
    float distanceToEdgeX = (normalizedX < 0.5f) ? normalizedX : (1.0f - normalizedX);
    float distanceToEdgeY = (normalizedY < 0.5f) ? normalizedY : (1.0f - normalizedY);
    return std::min(distanceToEdgeX, distanceToEdgeY) * borderWidth;
}

void ChunkManager::enableProceduralGeneration() {
    heightMap = nullptr;
    useRealHeightMap = false;
    regenerateWorld();
}

void ChunkManager::enableHeightMapGeneration(const std::string& heightMapPath) {
    try {
        HeightMap* hm = new HeightMap(heightMapPath);
        setHeightMap(hm);
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load heightmap: " << e.what() << "\n";
    }
}