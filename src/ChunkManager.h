// src/ChunkManager.h
#pragma once

#include "Chunk.h"
#include "HeightMap.h"
#include "NoiseGenerator.h"

#include <unordered_map>
#include <mutex>
#include <memory>
#include <future>
#include <vector>
#include <string>

class ChunkManager {
public:
    ChunkManager(int worldChunksX, int worldChunksY, int chunkSize, int tileSize);

    // HeightMap management
    void setHeightMap(std::shared_ptr<HeightMap> hm);
    void enableProceduralGeneration();
    void enableHeightMapGeneration(const std::string& heightMapPath);

    // Chunk generation and retrieval
    std::shared_ptr<Chunk> generateChunk(int chunkX, int chunkY);
    void addLoadedChunk(int chunkX, int chunkY, std::shared_ptr<Chunk> chunk);
    void unloadChunk(int chunkX, int chunkY);
    bool isChunkLoaded(int chunkX, int chunkY) const;
    std::shared_ptr<Chunk> getChunk(int chunkX, int chunkY) const;

    // Regeneration
    void regenerateChunk(int chunkX, int chunkY);
    void regenerateAllChunks();

    // Access loaded chunks for rendering
    std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>> getLoadedChunks() const;

    // Noise Layer Management
    const std::vector<NoiseLayer>& getNoiseLayers() const { return noiseGenerator.getNoiseLayers(); }

    void setNoiseLayerType(size_t index, FastNoiseLite::NoiseType type);
    void setNoiseLayerFrequency(size_t index, float frequency);
    void setNoiseLayerAmplitude(size_t index, float amplitude);
    void setNoiseLayerSeed(size_t index, int seed);
    void setNoiseLayerCellularDistanceFunction(size_t index, FastNoiseLite::CellularDistanceFunction distanceFunction);
    void setNoiseLayerCellularReturnType(size_t index, FastNoiseLite::CellularReturnType returnType);
    void setNoiseLayerCellularJitter(size_t index, float jitter);
    void addNoiseLayer(const NoiseLayer& layer);
    void removeLastNoiseLayer();

    // Land/Water Threshold Management
    float getLandThreshold() const { return landThreshold; }
    void setLandThreshold(float threshold);

    // Border Width Management
    float getBorderWidth() const { return borderWidth; }
    void setBorderWidth(float width);

    // Attenuation Factor Management
    float getAttenuationFactor() const { return attenuationFactor; }
    void setAttenuationFactor(float factor);

    // World dimensions getters
    int getWorldChunksX() const { return WORLD_CHUNKS_X; }
    int getWorldChunksY() const { return WORLD_CHUNKS_Y; }

private:
    int WORLD_CHUNKS_X;
    int WORLD_CHUNKS_Y;
    int CHUNK_SIZE;
    int TILE_SIZE;

    // Thread-safe loaded chunks storage
    std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>> loadedChunks;
    mutable std::mutex chunksMutex; // To protect loadedChunks

    // Noise generation
    NoiseGenerator noiseGenerator;

    // HeightMap
    std::shared_ptr<HeightMap> heightMap;
    bool useRealHeightMap = false;

    // World dimensions
    float worldWidth;
    float worldHeight;

    // Procedural generation parameters
    float landThreshold;
    float borderWidth;
    float attenuationFactor;

    // Helper methods
    std::shared_ptr<Chunk> generateChunkInternal(int chunkX, int chunkY);

    // Aggregate tiles for LOD
    sf::Color aggregateTiles(const std::vector<sf::Color>& tileColors, int startX, int startY, int size) const;

    // Define land, water, and boundary colors
    const sf::Color LAND_COLOR = sf::Color(231, 232, 234);      // Example land color
    const sf::Color WATER_COLOR = sf::Color(174, 223, 246);     // Example water color
    const sf::Color BOUNDARY_COLOR = sf::Color(200, 200, 200);  // Example boundary color
};
