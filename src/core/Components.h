#pragma once

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "../world/TerrainType.h"

const int CHUNK_SIZE_X = 32;
const int CHUNK_SIZE_Y = 32;

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
    ChunkComponent() : cells(CHUNK_SIZE_X * CHUNK_SIZE_Y, TerrainType::WATER) {}
};

struct WorldGridComponent {
    float cellSize = 16.0f;
    sf::Vector2i chunkDimensionsInCells = { CHUNK_SIZE_X, CHUNK_SIZE_Y };
};