// src/states/GameplayState.cpp
#include "GameplayState.h"
#include "../entities/EntityFactory.h"
#include "../systems/Renderer.h"
#include "../systems/TerrainGenerationSystem.h"
#include "../entities/LandDrawable.h"
#include "../entities/WaterDrawable.h"

GameplayState::GameplayState(Game& game)
    : State(game), renderer(std::make_shared<Renderer>()),
    terrainSystem(std::make_unique<TerrainGenerationSystem>(entityManager, worldWidth, worldHeight, scale)) {
    terrainSystem->generateTerrain();

    EntityFactory factory;

    // Initialize Renderer with terrain entities
    for (int x = 0; x < worldWidth; ++x) {
        for (int y = 0; y < worldHeight; ++y) {
            Entity entity = terrainSystem->terrainGrid[x][y];
            auto position = entityManager.getComponent<Position>(entity.getID());
            auto terrain = entityManager.getComponent<Terrain>(entity.getID());
            auto height = entityManager.getComponent<Height>(entity.getID());

            // Create a drawable entity based on terrain type
            std::shared_ptr<DrawableEntity> drawable;
            if (terrain->type == Terrain::Type::Land) {
                drawable = std::make_shared<LandDrawable>(position->x, position->y, height->value);
            }
            else {
                drawable = std::make_shared<WaterDrawable>(position->x, position->y);
            }

            renderer->addDrawable(drawable);
        }
    }
}
