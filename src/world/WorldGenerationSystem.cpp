#include "WorldGenerationSystem.h"
#include "../Logger.h"
#include "../core/Constants.h"

#include <random>
#include <algorithm>
#include <vector>

WorldGenerationSystem::WorldGenerationSystem(entt::registry& registry, EventBus& eventBus)
    : _registry(registry), _eventBus(eventBus) { // <-- MODIFIED
    // Connect to the event bus using the member variable
    _worldGenParamsListener = _eventBus.sink<WorldGenParamsChangeEvent>().connect<&WorldGenerationSystem::onWorldGenParamsChange>(this);
    LOG_INFO("WorldGenerationSystem", "System created and listening for world generation events.");
}

WorldGenerationSystem::~WorldGenerationSystem() {
    // Disconnect using the member variable
    _eventBus.sink<WorldGenParamsChangeEvent>().disconnect(this);
}

void WorldGenerationSystem::setParams(const WorldGenParams& params) {
    _params = params;
    configureNoise();
}

void WorldGenerationSystem::configureNoise() {
    _noiseGenerator.SetSeed(_params.seed);
    _noiseGenerator.SetFrequency(_params.frequency);
    _noiseGenerator.SetNoiseType(_params.noiseType);
    _noiseGenerator.SetFractalType(_params.fractalType);
    _noiseGenerator.SetFractalOctaves(_params.octaves);
    _noiseGenerator.SetFractalLacunarity(_params.lacunarity);
    _noiseGenerator.SetFractalGain(_params.gain);
}

const WorldGridComponent& WorldGenerationSystem::getWorldGridSettings() {
    auto view = _registry.view<WorldGridComponent>();
    if (view.empty()) {
        throw std::runtime_error("No WorldGridComponent found in the registry.");
    }
    return view.get<WorldGridComponent>(view.front());
}

sf::Vector2f WorldGenerationSystem::getWorldSize() {
    try {
        const WorldGridComponent& worldGrid = getWorldGridSettings();
        float worldWidth = static_cast<float>(worldGrid.worldDimensionsInChunks.x * worldGrid.chunkDimensionsInCells.x) * worldGrid.cellSize;
        float worldHeight = static_cast<float>(worldGrid.worldDimensionsInChunks.y * worldGrid.chunkDimensionsInCells.y) * worldGrid.cellSize;
        return {worldWidth, worldHeight};
    } catch (const std::runtime_error& e) {
        LOG_ERROR("WorldGenerationSystem", "Cannot get world size: %s", e.what());
        return {0.0f, 0.0f};
    }
}

bool isInside(const sf::Vector2f& point, const std::vector<sf::Vector2f>& polygon) {
    if (polygon.empty()) {
        return false;
    }
    int n = polygon.size();
    bool inside = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((polygon[i].y > point.y) != (polygon[j].y > point.y)) &&
            (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x)) {
            inside = !inside;
        }
    }
    return inside;
}

void WorldGenerationSystem::generateChunk(entt::registry& registry, entt::entity chunkEntity) {
    auto& chunk = registry.get<ChunkComponent>(chunkEntity);
    const auto& worldGrid = registry.get<WorldGridComponent>(registry.view<WorldGridComponent>().front());

    const int chunkCellSize = static_cast<int>(worldGrid.chunkDimensionsInCells.x); // CORRECTED
    chunk.cells.resize(chunkCellSize * chunkCellSize);

    for (int y = 0; y < chunkCellSize; ++y) {
        for (int x = 0; x < chunkCellSize; ++x) {
            chunk.cells[y * chunkCellSize + x] = TerrainType::WATER;
        }
    }

    chunk.isMeshDirty = true;
}

void WorldGenerationSystem::onWorldGenParamsChange(const WorldGenParamsChangeEvent& event) {
    LOG_INFO("WorldGenerationSystem", "World generation parameters updated.");
    setParams(event.params);

    auto view = _registry.view<WorldGridComponent>();
    if (!view.empty()) {
        auto& worldGrid = view.get<WorldGridComponent>(view.front());
        worldGrid.worldDimensionsInChunks = {event.worldChunksX, event.worldChunksY};
        worldGrid.chunkDimensionsInCells = {event.chunkSizeX, event.chunkSizeY};
        worldGrid.cellSize = event.cellSize;
    }
}