#include "ChunkManagerSystem.h"
#include "../core/Components.h"
#include "../Logger.h"

ChunkManagerSystem::ChunkManagerSystem(ServiceLocator& serviceLocator, WorldGenerationSystem& worldGenSystem)
    : _serviceLocator(serviceLocator),
      _worldGenSystem(worldGenSystem),
      _registry(*serviceLocator.registry),
      _activeChunks() {}

void ChunkManagerSystem::update(sf::Time dt) {
    const auto& camera = *_serviceLocator.camera;
    const auto& worldGrid = _registry.get<WorldGridComponent>(_registry.view<WorldGridComponent>().front());

    sf::Vector2f cameraCenter = camera.getCenter();
    float cellSize = worldGrid.cellSize;
    const auto& chunkDims = worldGrid.chunkDimensionsInCells;

    sf::Vector2i centerChunk = {
        static_cast<int>(cameraCenter.x / (chunkDims.x * cellSize)),
        static_cast<int>(cameraCenter.y / (chunkDims.y * cellSize))
    };

    std::set<sf::Vector2i, Vector2iCompare> requiredChunks;
    for (int y = centerChunk.y - _viewDistance; y <= centerChunk.y + _viewDistance; ++y) {
        for (int x = centerChunk.x - _viewDistance; x <= centerChunk.x + _viewDistance; ++x) {
            requiredChunks.insert({x, y});
        }
    }

    // Identify chunks to unload without modifying the set
    std::vector<sf::Vector2i> chunksToUnload;
    for (const auto& chunkPos : _activeChunks) {
        if (requiredChunks.find(chunkPos) == requiredChunks.end()) {
            chunksToUnload.push_back(chunkPos);
        }
    }

    // Unload the identified chunks
    for (const auto& chunkPos : chunksToUnload) {
        unloadChunk(chunkPos);
    }

    // Load new chunks
    for (const auto& chunkPos : requiredChunks) {
        if (_activeChunks.find(chunkPos) == _activeChunks.end()) {
            loadChunk(chunkPos);
        }
    }
}

void ChunkManagerSystem::loadChunk(const sf::Vector2i& chunkPos) {
    if (_activeChunks.count(chunkPos)) return;

    const auto& worldGrid = _registry.get<WorldGridComponent>(_registry.view<WorldGridComponent>().front());
    const auto& worldDims = worldGrid.worldDimensionsInChunks;

    if (chunkPos.x < 0 || chunkPos.x >= worldDims.x || chunkPos.y < 0 || chunkPos.y >= worldDims.y) {
        return; // Out of world bounds
    }

    auto entity = _registry.create();
    auto& chunk = _registry.emplace<ChunkComponent>(entity, worldGrid.chunkDimensionsInCells.x, worldGrid.chunkDimensionsInCells.y);
    chunk.chunkGridPosition = chunkPos;
    
    _worldGenSystem.generateChunk(_registry, entity);
    _activeChunks.insert(chunkPos);
    LOG_INFO("ChunkManagerSystem", "Loaded chunk at (%d, %d)", chunkPos.x, chunkPos.y);
}

void ChunkManagerSystem::unloadChunk(const sf::Vector2i& chunkPos) {
    for (auto entity : _registry.view<ChunkComponent>()) {
        const auto& chunk = _registry.get<ChunkComponent>(entity);
        if (chunk.chunkGridPosition == chunkPos) {
            _registry.destroy(entity);
            _activeChunks.erase(chunkPos);
            LOG_INFO("ChunkManagerSystem", "Unloaded chunk at (%d, %d)", chunkPos.x, chunkPos.y);
            return;
        }
    }
}
