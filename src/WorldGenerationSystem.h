#pragma once

#include "WorldGrid.h"
#include "Components.h"

class WorldGenerationSystem {
public:
    WorldGenerationSystem(WorldGrid& worldGrid);
    void generateWorld(int worldWidth, int worldHeight, unsigned int seed = 0);
    void initializeGrid(int worldWidth, int worldHeight);
    void generateIslands(int worldWidth, int worldHeight, unsigned int seed);
    void generateRivers(int worldWidth, int worldHeight, unsigned int seed);
    WorldGrid& _worldGrid;
};