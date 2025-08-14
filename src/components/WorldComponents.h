#pragma once

#include "world/TerrainType.h"
#include "world/WorldData.h"
#include <SFML/System.hpp>
#include <vector>

// Defines the levels of detail for chunk rendering.
enum class LODLevel { LOD0, LOD1, LOD2, LOD3, Count };

// Represents a single cell in the world grid.
struct GridCellComponent {
    TerrainType type = TerrainType::WATER;
};

// A tag to mark chunks that are currently being loaded.
struct ChunkLoadingTag {};

// The position of a chunk in the world grid.
struct ChunkPositionComponent {
    sf::Vector2i chunkGridPosition;
};

// The terrain data for a chunk.
struct ChunkTerrainComponent {
    std::vector<TerrainType> cells;
};

// The noise values used to generate the terrain for a chunk.
struct ChunkNoiseComponent {
    std::vector<float> noiseValues;
    std::vector<float> rawNoiseValues;
};

// The current state of a chunk, such as whether its mesh needs rebuilding.
struct ChunkStateComponent {
    bool isMeshDirty = true;
    LODLevel lodLevel = LODLevel::LOD0;
};

// Defines the overall structure of the world grid.
struct WorldGridComponent {
    sf::Vector2i worldDimensionsInChunks = {100, 100};
    sf::Vector2i chunkDimensionsInCells = {32, 32};
    float cellSize = 16.0f;
};

// Holds the parameters for world generation.
struct WorldStateComponent {
    WorldGenParams activeParams;
    WorldGenParams pendingParams;
    WorldGenParams generatingParams;
};
