// src/ChunkManager.cpp
#include "ChunkManager.h"

#include <cmath>
#include <iostream>
#include <algorithm>

// Default procedural generation parameters
constexpr float DEFAULT_LAND_THRESHOLD = 0.0f;
constexpr float DEFAULT_BORDER_WIDTH = 4.35f;
constexpr float DEFAULT_ATTENUATION_FACTOR = 0.243f;
constexpr int AGGREGATION_FACTORS[] = { 1, 2, 4, 8, 16 }; // LOD0 to LOD4
constexpr int NUM_LODS = sizeof(AGGREGATION_FACTORS) / sizeof(int);

ChunkManager::ChunkManager(int worldChunksX, int worldChunksY, int chunkSize, int tileSize)
    : WORLD_CHUNKS_X(worldChunksX), WORLD_CHUNKS_Y(worldChunksY),
    CHUNK_SIZE(chunkSize), TILE_SIZE(tileSize),
    landThreshold(DEFAULT_LAND_THRESHOLD), borderWidth(DEFAULT_BORDER_WIDTH),
    attenuationFactor(DEFAULT_ATTENUATION_FACTOR),
    heightMap(nullptr),
    useRealHeightMap(false),
    worldWidth(static_cast<float>(WORLD_CHUNKS_X* CHUNK_SIZE* TILE_SIZE)),
    worldHeight(static_cast<float>(WORLD_CHUNKS_Y* CHUNK_SIZE* TILE_SIZE))
{
    noiseGenerator.initializeDefaultLayers();
}

void ChunkManager::setHeightMap(std::shared_ptr<HeightMap> hm) {
    std::vector<ChunkCoord> coordsToRegenerate;

    {
        std::lock_guard<std::mutex> lock(chunksMutex);
        heightMap = hm;
        useRealHeightMap = (heightMap != nullptr);

        // Collect the coordinates of loaded chunks while holding the lock
        for (const auto& [coord, chunk] : loadedChunks) {
            coordsToRegenerate.push_back(coord);
        }
    } // Release the lock here

    // Regenerate chunks outside the locked section to prevent deadlock
    for (const ChunkCoord& coord : coordsToRegenerate) {
        regenerateChunk(coord.x, coord.y);
    }
}

void ChunkManager::enableProceduralGeneration() {
    setHeightMap(nullptr);
}

void ChunkManager::enableHeightMapGeneration(const std::string& heightMapPath) {
    try {
        auto hm = std::make_shared<HeightMap>(heightMapPath);
        setHeightMap(hm);
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load heightmap: " << e.what() << "\nProceeding with procedural generation.\n";
        enableProceduralGeneration();
    }
}

std::shared_ptr<Chunk> ChunkManager::generateChunk(int chunkX, int chunkY) {
    // Clamp chunk indices to stay within world boundaries
    chunkX = std::clamp(chunkX, 0, WORLD_CHUNKS_X - 1);
    chunkY = std::clamp(chunkY, 0, WORLD_CHUNKS_Y - 1);

    auto chunk = std::make_shared<Chunk>();
    int totalTilesX = WORLD_CHUNKS_X * CHUNK_SIZE;
    int totalTilesY = WORLD_CHUNKS_Y * CHUNK_SIZE;

    // Calculate tile size in float for positioning
    float tileSizeF = static_cast<float>(TILE_SIZE);

    std::vector<float> tileHeights(CHUNK_SIZE * CHUNK_SIZE, 0.0f);
    std::vector<sf::Color> tileColors(CHUNK_SIZE * CHUNK_SIZE, sf::Color::Black);

    // Generate tile data for LOD0
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
                float invScaleX = (static_cast<float>(heightMap->getWidth()) - 1.0f) / (static_cast<float>(totalTilesX) - 1.0f);
                float invScaleY = (static_cast<float>(heightMap->getHeight()) - 1.0f) / (static_cast<float>(totalTilesY) - 1.0f);
                height = heightMap->getScaledHeight(static_cast<float>(tileX), static_cast<float>(tileY), invScaleX, invScaleY);
            }
            else {
                float normalizedX = worldX / worldWidth;
                float normalizedY = worldY / worldHeight;

                float edgeDistance = std::sqrt(normalizedX * normalizedX + normalizedY * normalizedY);
                float edgeAttenuationFactor = std::pow(edgeDistance, attenuationFactor);

                height = noiseGenerator.generateHeight(worldX, worldY);
                height *= edgeAttenuationFactor;
            }

            sf::Color color = (height > landThreshold) ? sf::Color(231, 232, 234) : sf::Color(174, 223, 246);

            tileHeights[y * CHUNK_SIZE + x] = height;
            tileColors[y * CHUNK_SIZE + x] = color;

            // Define the four corners of the quad
            sf::Vertex topLeft(sf::Vector2f(worldX, worldY), color);
            sf::Vertex topRight(sf::Vector2f(worldX + tileSizeF, worldY), color);
            sf::Vertex bottomRight(sf::Vector2f(worldX + tileSizeF, worldY + tileSizeF), color);
            sf::Vertex bottomLeft(sf::Vector2f(worldX, worldY + tileSizeF), color);

            // Append the quad to the LOD0 vertex array
            chunk->verticesLOD0.append(topLeft);
            chunk->verticesLOD0.append(topRight);
            chunk->verticesLOD0.append(bottomRight);
            chunk->verticesLOD0.append(bottomLeft);
        }
    }

    // Generate vertex arrays for each LOD
    for (size_t lod = 1; lod < NUM_LODS; ++lod) {
        int factor = AGGREGATION_FACTORS[lod];
        // Ensure that the factor divides the chunk size
        if (CHUNK_SIZE % factor != 0) {
            std::cerr << "Aggregation factor " << factor << " does not divide chunk size " << CHUNK_SIZE << ". Skipping LOD" << lod << ".\n";
            continue;
        }

        for (int y = 0; y < CHUNK_SIZE; y += factor) {
            for (int x = 0; x < CHUNK_SIZE; x += factor) {
                sf::Color aggregatedColor = aggregateTiles(tileColors, x, y, factor);

                float aggWorldX = static_cast<float>((chunkX * CHUNK_SIZE + x) * TILE_SIZE);
                float aggWorldY = static_cast<float>((chunkY * CHUNK_SIZE + y) * TILE_SIZE);
                float aggSizeF = static_cast<float>(factor * TILE_SIZE);

                sf::Vertex aggTopLeft(sf::Vector2f(aggWorldX, aggWorldY), aggregatedColor);
                sf::Vertex aggTopRight(sf::Vector2f(aggWorldX + aggSizeF, aggWorldY), aggregatedColor);
                sf::Vertex aggBottomRight(sf::Vector2f(aggWorldX + aggSizeF, aggWorldY + aggSizeF), aggregatedColor);
                sf::Vertex aggBottomLeft(sf::Vector2f(aggWorldX, aggWorldY + aggSizeF), aggregatedColor);

                // Append to the appropriate LOD vertex array
                switch (lod) {
                case 1:
                    chunk->verticesLOD1.append(aggTopLeft);
                    chunk->verticesLOD1.append(aggTopRight);
                    chunk->verticesLOD1.append(aggBottomRight);
                    chunk->verticesLOD1.append(aggBottomLeft);
                    break;
                case 2:
                    chunk->verticesLOD2.append(aggTopLeft);
                    chunk->verticesLOD2.append(aggTopRight);
                    chunk->verticesLOD2.append(aggBottomRight);
                    chunk->verticesLOD2.append(aggBottomLeft);
                    break;
                case 3:
                    chunk->verticesLOD3.append(aggTopLeft);
                    chunk->verticesLOD3.append(aggTopRight);
                    chunk->verticesLOD3.append(aggBottomRight);
                    chunk->verticesLOD3.append(aggBottomLeft);
                    break;
                case 4:
                    chunk->verticesLOD4.append(aggTopLeft);
                    chunk->verticesLOD4.append(aggTopRight);
                    chunk->verticesLOD4.append(aggBottomRight);
                    chunk->verticesLOD4.append(aggBottomLeft);
                    break;
                default:
                    // Handle additional LODs if necessary
                    break;
                }
            }
        }
    }

    return chunk;
}

void ChunkManager::addLoadedChunk(int chunkX, int chunkY, std::shared_ptr<Chunk> chunk) {
    ChunkCoord coord{ chunkX, chunkY };
    std::lock_guard<std::mutex> lock(chunksMutex);
    loadedChunks[coord] = chunk;
}

void ChunkManager::unloadChunk(int chunkX, int chunkY) {
    ChunkCoord coord{ chunkX, chunkY };
    std::lock_guard<std::mutex> lock(chunksMutex);
    loadedChunks.erase(coord);
}

bool ChunkManager::isChunkLoaded(int chunkX, int chunkY) const {
    ChunkCoord coord{ chunkX, chunkY };
    std::lock_guard<std::mutex> lock(chunksMutex);
    return loadedChunks.find(coord) != loadedChunks.end();
}

std::shared_ptr<Chunk> ChunkManager::getChunk(int chunkX, int chunkY) const {
    ChunkCoord coord{ chunkX, chunkY };
    std::lock_guard<std::mutex> lock(chunksMutex);
    auto it = loadedChunks.find(coord);
    if (it != loadedChunks.end()) {
        return it->second;
    }
    else {
        std::cerr << "Warning: Attempted to access unloaded chunk (" << chunkX << ", " << chunkY << ").\n";
        return nullptr;
    }
}

void ChunkManager::regenerateChunk(int chunkX, int chunkY) {
    ChunkCoord coord{ chunkX, chunkY };
    {
        std::lock_guard<std::mutex> lock(chunksMutex);
        if (loadedChunks.find(coord) != loadedChunks.end()) {
            loadedChunks[coord] = generateChunkInternal(chunkX, chunkY);
        }
    }
}

void ChunkManager::regenerateAllChunks() {
    std::vector<ChunkCoord> coordsToRegenerate;

    {
        std::lock_guard<std::mutex> lock(chunksMutex);
        for (const auto& [coord, chunk] : loadedChunks) {
            coordsToRegenerate.push_back(coord);
        }
    } // Release the lock here

    for (const ChunkCoord& coord : coordsToRegenerate) {
        regenerateChunk(coord.x, coord.y);
    }
}

std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>> ChunkManager::getLoadedChunks() const {
    std::lock_guard<std::mutex> lock(chunksMutex);
    return loadedChunks;
}

// Noise Layer Management Methods
void ChunkManager::setNoiseLayerType(size_t index, FastNoiseLite::NoiseType type) {
    if (index < noiseGenerator.getNoiseLayers().size()) {
        noiseGenerator.setNoiseLayerType(index, type);
    }
}

void ChunkManager::setNoiseLayerFrequency(size_t index, float frequency) {
    if (index < noiseGenerator.getNoiseLayers().size()) {
        noiseGenerator.setNoiseLayerFrequency(index, frequency);
    }
}

void ChunkManager::setNoiseLayerAmplitude(size_t index, float amplitude) {
    if (index < noiseGenerator.getNoiseLayers().size()) {
        noiseGenerator.setNoiseLayerAmplitude(index, amplitude);
    }
}

void ChunkManager::setNoiseLayerSeed(size_t index, int seed) {
    if (index < noiseGenerator.getNoiseLayers().size()) {
        noiseGenerator.setNoiseLayerSeed(index, seed);
    }
}

void ChunkManager::setNoiseLayerCellularDistanceFunction(size_t index, FastNoiseLite::CellularDistanceFunction distanceFunction) {
    if (index < noiseGenerator.getNoiseLayers().size()) {
        noiseGenerator.setNoiseLayerCellularDistanceFunction(index, distanceFunction);
    }
}

void ChunkManager::setNoiseLayerCellularReturnType(size_t index, FastNoiseLite::CellularReturnType returnType) {
    if (index < noiseGenerator.getNoiseLayers().size()) {
        noiseGenerator.setNoiseLayerCellularReturnType(index, returnType);
    }
}

void ChunkManager::setNoiseLayerCellularJitter(size_t index, float jitter) {
    if (index < noiseGenerator.getNoiseLayers().size()) {
        noiseGenerator.setNoiseLayerCellularJitter(index, jitter);
    }
}

void ChunkManager::addNoiseLayer(const NoiseLayer& layer) {
    noiseGenerator.addNoiseLayer(layer);
}

void ChunkManager::removeLastNoiseLayer() {
    noiseGenerator.removeLastNoiseLayer();
}

// Land/Water Threshold Management
void ChunkManager::setLandThreshold(float threshold) {
    landThreshold = threshold;
}

// Border Width Management
void ChunkManager::setBorderWidth(float width) {
    borderWidth = width;
}

// Attenuation Factor Management
void ChunkManager::setAttenuationFactor(float factor) {
    attenuationFactor = factor;
}

// Internal chunk generation logic
std::shared_ptr<Chunk> ChunkManager::generateChunkInternal(int chunkX, int chunkY) {
    return generateChunk(chunkX, chunkY);
}

sf::Color ChunkManager::aggregateTiles(const std::vector<sf::Color>& tileColors, int startX, int startY, int size) const {
    int landCount = 0;
    int waterCount = 0;

    // Iterate through the specified block
    for (int y = startY; y < startY + size && y < CHUNK_SIZE; ++y) {
        for (int x = startX; x < startX + size && x < CHUNK_SIZE; ++x) {
            sf::Color currentColor = tileColors[y * CHUNK_SIZE + x];

            if (currentColor == LAND_COLOR) {
                landCount++;
            }
            else if (currentColor == WATER_COLOR) {
                waterCount++;
            }
        }
    }

    int totalTiles = size * size;

    // Determine the aggregated color based on tile composition
    if (landCount == totalTiles) {
        return LAND_COLOR;
    }
    else if (waterCount == totalTiles) {
        return WATER_COLOR;
    }
    else {
        return BOUNDARY_COLOR;
    }
}
