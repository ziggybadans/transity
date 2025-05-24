#include "WorldGenerationSystem.h"
#include <iostream>

WorldGenerationSystem::WorldGenerationSystem() {
    _noiseGenerator.SetSeed(1337);
    _noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    _noiseGenerator.SetFrequency(0.01f);
    _noiseGenerator.SetFractalType(FastNoiseLite::FractalType_FBm);
    _noiseGenerator.SetFractalOctaves(5);
    _noiseGenerator.SetFractalLacunarity(2.0f);
    _noiseGenerator.SetFractalGain(0.5f);
}

void WorldGenerationSystem::configureNoise(
    int seed, 
    float frequency, 
    FastNoiseLite::NoiseType noiseType,
    FastNoiseLite::FractalType fractalType,
    int octaves,
    float lacunarity,
    float gain) {
    
    _noiseGenerator.SetSeed(seed);
    _noiseGenerator.SetFrequency(frequency);
    _noiseGenerator.SetNoiseType(noiseType);
    _noiseGenerator.SetFractalType(fractalType);
    _noiseGenerator.SetFractalOctaves(octaves);
    _noiseGenerator.SetFractalLacunarity(lacunarity);
    _noiseGenerator.SetFractalGain(gain);
}

const WorldGridComponent& WorldGenerationSystem::getWorldGridSettings(entt::registry& registry) {
    auto view = registry.view<WorldGridComponent>();
    if (view.empty()) {
        throw std::runtime_error("No WorldGridComponent found in the registry.");
    }
    return view.get<WorldGridComponent>(view.front());
}

// Implementation for generateChunk
void WorldGenerationSystem::generateChunk(entt::registry& registry, entt::entity chunkEntity) {
    if (!registry.all_of<ChunkComponent>(chunkEntity)) {
        // Log error or handle: entity does not have a ChunkComponent
        std::cerr << "Error: Entity does not have a ChunkComponent for generation." << std::endl;
        return;
    }

    ChunkComponent& chunk = registry.get<ChunkComponent>(chunkEntity);
    const WorldGridComponent& worldGrid = getWorldGridSettings(registry);

    // Ensure cells vector is correctly sized (constructor should handle this)
    // chunk.cells.resize(worldGrid.chunkDimensionsInCells.x * worldGrid.chunkDimensionsInCells.y);

    std::cout << "Generating chunk at: (" << chunk.chunkGridPosition.x << ", " << chunk.chunkGridPosition.y << ")" << std::endl;

    for (int y = 0; y < worldGrid.chunkDimensionsInCells.y; ++y) {
        for (int x = 0; x < worldGrid.chunkDimensionsInCells.x; ++x) {
            float worldX = (chunk.chunkGridPosition.x * worldGrid.chunkDimensionsInCells.x + x) * worldGrid.cellSize + (worldGrid.cellSize / 2.0f);
            float worldY = (chunk.chunkGridPosition.y * worldGrid.chunkDimensionsInCells.y + y) * worldGrid.cellSize + (worldGrid.cellSize / 2.0f);

            float noiseValue = _noiseGenerator.GetNoise(worldX, worldY);
            int cellIndex = y * worldGrid.chunkDimensionsInCells.x + x;
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
void WorldGenerationSystem::generateWorld(entt::registry& registry, int numChunksX, int numChunksY) {
    // Ensure WorldGridComponent exists
    entt::entity worldGridEntity = entt::null;
    auto worldView = registry.view<WorldGridComponent>();
    if (worldView.empty()) {
        worldGridEntity = registry.create();
        registry.emplace<WorldGridComponent>(worldGridEntity); // Default settings
        std::cout << "Created WorldGridComponent entity." << std::endl;
    } else {
        worldGridEntity = worldView.front();
    }
    
    std::cout << "Generating initial world of " << numChunksX << "x" << numChunksY << " chunks." << std::endl;

    for (int cy = 0; cy < numChunksY; ++cy) {
        for (int cx = 0; cx < numChunksX; ++cx) {
            entt::entity newChunkEntity = registry.create();
            ChunkComponent& chunkComp = registry.emplace<ChunkComponent>(newChunkEntity);
            chunkComp.chunkGridPosition = {cx, cy};
            // The ChunkComponent constructor already initializes cells to WATER.
            
            // Add PositionComponent for the chunk entity itself if needed for spatial queries or rendering chunk boundaries
            // registry.emplace<PositionComponent>(newChunkEntity, sf::Vector2f{cx * CHUNK_SIZE_X * worldGrid.cellSize, cy * CHUNK_SIZE_Y * worldGrid.cellSize});

            generateChunk(registry, newChunkEntity);
        }
    }
    std::cout << "Initial world generation finished." << std::endl;
}