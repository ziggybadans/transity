#include "WorldGenerationSystem.h"
#include <iostream>

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

// Implementation for generateChunk
void WorldGenerationSystem::generateChunk(entt::entity chunkEntity) {
    if (!_registry.all_of<ChunkComponent>(chunkEntity)) {
        // Log error or handle: entity does not have a ChunkComponent
        std::cerr << "Error: Entity does not have a ChunkComponent for generation." << std::endl;
        return;
    }

    ChunkComponent& chunk = _registry.get<ChunkComponent>(chunkEntity);
    const WorldGridComponent& worldGrid = getWorldGridSettings();

    // Ensure cells vector is correctly sized (constructor should handle this)
    // chunk.cells.resize(worldGrid.chunkDimensionsInCells.x * worldGrid.chunkDimensionsInCells.y);

    std::cout << "Generating chunk at: (" << chunk.chunkGridPosition.x << ", " << chunk.chunkGridPosition.y << ")" << std::endl;

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
    std::cout << "Chunk generation complete for: (" << chunk.chunkGridPosition.x << ", " << chunk.chunkGridPosition.y << ")" << std::endl;
}

// Implementation for generateInitialWorld (Example)
void WorldGenerationSystem::generateWorld(int numChunksX, int numChunksY) {
    // Ensure WorldGridComponent exists
    entt::entity worldGridEntity = entt::null;
    auto worldView = _registry.view<WorldGridComponent>();
    if (worldView.empty()) {
        worldGridEntity = _registry.create();
        _registry.emplace<WorldGridComponent>(worldGridEntity); // Default settings
        std::cout << "Created WorldGridComponent entity." << std::endl;
    } else {
        worldGridEntity = worldView.front();
    }

    std::cout << "Clearing existing chunk entities..." << std::endl;
    auto chunkView = _registry.view<ChunkComponent>();
    for (auto entity : chunkView) {
        _registry.destroy(entity);
    }
    std::cout << "Existing chunk entities cleared." << std::endl;
    
    std::cout << "Generating world of " << numChunksX << "x" << numChunksY << " chunks." << std::endl;

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
    std::cout << "Initial world generation finished." << std::endl;
}