// src/systems/TerrainGenerationSystem.h
#pragma once
#include "../components/Component.h"
#include "../entities/EntityManager.h"
#include "../utils/FastNoiseLite.h"
#include <vector>
#include <memory>

class TerrainGenerationSystem {
public:
    TerrainGenerationSystem(EntityManager& entityManager, int worldWidth, int worldHeight, float scale);
    void generateTerrain();

    // Expose terrainGrid for external access (e.g., GameplayState)
    std::vector<std::vector<Entity>> terrainGrid;

private:
    EntityManager& entityManager;
    int worldWidth;
    int worldHeight;
    float scale;
    FastNoiseLite noise;
};
