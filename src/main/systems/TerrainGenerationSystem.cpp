// src/systems/TerrainGenerationSystem.cpp
#include "TerrainGenerationSystem.h"
#include "../entities/EntityFactory.h"

TerrainGenerationSystem::TerrainGenerationSystem(EntityManager& entityManager, int worldWidth, int worldHeight, float scale)
    : entityManager(entityManager), worldWidth(worldWidth), worldHeight(worldHeight), scale(scale) {
    noise.SetSeed(1337);
    noise.SetFrequency(0.01f);
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    // Configure additional noise settings as needed
}

void TerrainGenerationSystem::generateTerrain() {
    EntityFactory factory;

    terrainGrid.resize(worldWidth, std::vector<Entity>(worldHeight, Entity(0)));

    for (int x = 0; x < worldWidth; ++x) {
        for (int y = 0; y < worldHeight; ++y) {
            float noiseValue = noise.GetNoise(x * scale, y * scale);
            Terrain::Type type = (noiseValue > 0.0f) ? Terrain::Type::Land : Terrain::Type::Water;
            float height = (type == Terrain::Type::Land) ? noiseValue : 0.0f;

            Entity terrain = factory.createTerrain(entityManager, static_cast<float>(x), static_cast<float>(y), type, height);
            terrainGrid[x][y] = terrain;
        }
    }
}
