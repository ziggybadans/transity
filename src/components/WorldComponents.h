#pragma once

#include "world/TerrainType.h"
#include "world/WorldData.h"
#include <SFML/System.hpp>
#include <vector>

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

// The elevation values for a chunk, stored per cell in world units.
struct ChunkElevationComponent {
    std::vector<float> elevations;
};

// The current state of a chunk, such as whether its mesh needs rebuilding.
struct ChunkStateComponent {
    bool isMeshDirty = true;
};

// Defines the overall structure of the world grid.
struct WorldGridComponent {};

// Holds the parameters for world generation.
struct WorldStateComponent {
    WorldGenParams activeParams;
    WorldGenParams pendingParams;
    WorldGenParams generatingParams;
};
