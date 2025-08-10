#pragma once

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <set>

#include "../world/TerrainType.h"
#include "../world/WorldData.h"

enum class LODLevel {
    LOD0, // Highest detail
    LOD1,
    LOD2,
    LOD3, // Lowest detail
    Count // Add this to get the number of LOD levels
};

struct PositionComponent {
    sf::Vector2f coordinates;
};

struct RenderableComponent {
    sf::CircleShape shape;
    int zOrder;
};

struct StationComponent {
    std::vector<entt::entity> connectedLines;
};

struct LineComponent {
    sf::Color color;
    std::vector<entt::entity> stops;
    std::vector<sf::Vector2f> pathPoints;
    float thickness = 5.0f;
};

struct ClickableComponent {
    float boundingRadius;
};

struct ActiveLineStationTag {
    int order = 0;
};

struct GridCellComponent {
    TerrainType type = TerrainType::WATER;
};

struct ChunkComponent {
    sf::Vector2i chunkGridPosition;
    std::vector<TerrainType> cells;
    std::vector<float> noiseValues;
    std::vector<float> rawNoiseValues;
    std::vector<sf::VertexArray> lodVertexArrays;
    bool isMeshDirty = true;
    std::set<int> dirtyCells;
    LODLevel lodLevel = LODLevel::LOD0;

    ChunkComponent(int chunkWidth, int chunkHeight) : cells(chunkWidth * chunkHeight, TerrainType::WATER),
        noiseValues(chunkWidth * chunkHeight, 0.0f) {
        lodVertexArrays.resize(static_cast<size_t>(LODLevel::Count)); // Resize the vector
        for (auto& va : lodVertexArrays) {
            va.setPrimitiveType(sf::PrimitiveType::Triangles);
        }
    }
};

struct WorldGridComponent {
    sf::Vector2i worldDimensionsInChunks = { 100, 100 };
    sf::Vector2i chunkDimensionsInCells = { 32, 32 };
    float cellSize = 16.0f;
};

struct WorldStateComponent {
    WorldGenParams activeParams;
    WorldGenParams pendingParams;
    WorldGenParams generatingParams;
};