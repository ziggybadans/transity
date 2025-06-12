#include "WorldGenerationSystem.h"
#include "../Logger.h"

WorldGenerationSystem::WorldGenerationSystem(entt::registry& registry) : _registry(registry) {
    _seed = 1337;
    _frequency = 0.01f;
    _noiseType = FastNoiseLite::NoiseType_OpenSimplex2;
    _fractalType = FastNoiseLite::FractalType_FBm;
    _octaves = 5;
    _lacunarity = 2.0f;
    _gain = 0.5f;

    configureNoise();
}

void WorldGenerationSystem::configureNoise() {
    _noiseGenerator.SetSeed(_seed);
    _noiseGenerator.SetFrequency(_frequency);
    _noiseGenerator.SetNoiseType(_noiseType);
    _noiseGenerator.SetFractalType(_fractalType);
    _noiseGenerator.SetFractalOctaves(_octaves);
    _noiseGenerator.SetFractalLacunarity(_lacunarity);
    _noiseGenerator.SetFractalGain(_gain);

    generateWorld(3, 3);
}

const WorldGridComponent& WorldGenerationSystem::getWorldGridSettings() {
    auto view = _registry.view<WorldGridComponent>();
    if (view.empty()) {
        throw std::runtime_error("No WorldGridComponent found in the registry.");
    }
    return view.get<WorldGridComponent>(view.front());
}

void WorldGenerationSystem::generateChunk(entt::entity chunkEntity) {
    if (!_registry.all_of<ChunkComponent>(chunkEntity)) {
        LOG_ERROR("WorldGenerationSystem", "Error: Entity does not have a ChunkComponent for generation.");
        return;
    }

    ChunkComponent& chunk = _registry.get<ChunkComponent>(chunkEntity);
    const WorldGridComponent& worldGrid = getWorldGridSettings();

    // Ensure cells vector is correctly sized (constructor should handle this)
    // chunk.cells.resize(worldGrid.chunkDimensionsInCells.x * worldGrid.chunkDimensionsInCells.y);

    LOG_INFO("WorldGenerationSystem", "Generating chunk at: (%d, %d)", chunk.chunkGridPosition.x, chunk.chunkGridPosition.y);

    for (int y = 0; y < worldGrid.chunkDimensionsInCells.y; ++y) {
        for (int x = 0; x < worldGrid.chunkDimensionsInCells.x; ++x) {
            float worldX = (chunk.chunkGridPosition.x * worldGrid.chunkDimensionsInCells.x + x) * worldGrid.cellSize + (worldGrid.cellSize / 2.0f);
            float worldY = (chunk.chunkGridPosition.y * worldGrid.chunkDimensionsInCells.y + y) * worldGrid.cellSize + (worldGrid.cellSize / 2.0f);

            float noiseValue = _noiseGenerator.GetNoise(worldX, worldY);
            int cellIndex = y * worldGrid.chunkDimensionsInCells.x + x;

            chunk.noiseValues[cellIndex] = noiseValue;

            float landThreshold = -0.0001f;

            if (noiseValue > landThreshold) {
                chunk.cells[cellIndex] = TerrainType::LAND;
            } else {
                chunk.cells[cellIndex] = TerrainType::WATER;
            }
        }
    }
    // Optionally, mark the chunk as generated, e.g., by adding a GeneratedTag or setting a flag.
    // registry.emplace_or_replace<ChunkGeneratedTag>(chunkEntity);
    LOG_INFO("WorldGenerationSystem", "Chunk generation complete for: (%d, %d)", chunk.chunkGridPosition.x, chunk.chunkGridPosition.y);
}

void WorldGenerationSystem::generateWorld(int numChunksX, int numChunksY) {
    entt::entity worldGridEntity = entt::null;
    auto worldView = _registry.view<WorldGridComponent>();
    if (worldView.empty()) {
        worldGridEntity = _registry.create();
        _registry.emplace<WorldGridComponent>(worldGridEntity);
        LOG_INFO("WorldGenerationSystem", "Created WorldGridComponent entity.");
    } else {
        worldGridEntity = worldView.front();
    }

    LOG_INFO("WorldGenerationSystem", "Clearing existing chunk entities...");
    auto chunkView = _registry.view<ChunkComponent>();
    for (auto entity : chunkView) {
        _registry.destroy(entity);
    }
    LOG_INFO("WorldGenerationSystem", "Existing chunk entities cleared.");
    
    LOG_INFO("WorldGenerationSystem", "Generating world of %dx%d chunks.", numChunksX, numChunksY);

    for (int cy = 0; cy < numChunksY; ++cy) {
        for (int cx = 0; cx < numChunksX; ++cx) {
            entt::entity newChunkEntity = _registry.create();
            ChunkComponent& chunkComp = _registry.emplace<ChunkComponent>(newChunkEntity);
            chunkComp.chunkGridPosition = {cx, cy};
            // The ChunkComponent constructor already initializes cells to WATER.
            
            // Add PositionComponent for the chunk entity itself if needed for spatial queries or rendering chunk boundaries
            // registry.emplace<PositionComponent>(newChunkEntity, sf::Vector2f{cx * CHUNK_SIZE_X * worldGrid.cellSize, cy * CHUNK_SIZE_Y * worldGrid.cellSize});

            generateChunk(newChunkEntity);
        }
    }
    LOG_INFO("WorldGenerationSystem", "Initial world generation finished.");
}