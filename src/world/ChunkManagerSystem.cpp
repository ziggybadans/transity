#include "ChunkManagerSystem.h"
#include "../core/Components.h"
#include "../Logger.h"

ChunkManagerSystem::ChunkManagerSystem(ServiceLocator& serviceLocator, WorldGenerationSystem& worldGenSystem, EventBus& eventBus)
    : _serviceLocator(serviceLocator),
      _worldGenSystem(worldGenSystem),
      _registry(*serviceLocator.registry),
      _eventBus(eventBus),
      _activeChunks() {
    _regenerateWorldListener = _eventBus.sink<RegenerateWorldRequestEvent>().connect<&ChunkManagerSystem::onRegenerateWorld>(this);
    _swapWorldStateListener = _eventBus.sink<SwapWorldStateEvent>().connect<&ChunkManagerSystem::onSwapWorldState>(this);

    // Create the world state entity
    auto entity = _registry.create();
    _registry.emplace<WorldStateComponent>(entity);
}

ChunkManagerSystem::~ChunkManagerSystem() {
    _eventBus.sink<RegenerateWorldRequestEvent>().disconnect(this);
    _eventBus.sink<SwapWorldStateEvent>().disconnect(this);
}

void ChunkManagerSystem::onRegenerateWorld(const RegenerateWorldRequestEvent& event) {
    if (_generationFuture.valid()) {
        LOG_WARN("ChunkManagerSystem", "World regeneration is already in progress.");
        return;
    }

    LOG_INFO("ChunkManagerSystem", "Regenerating world.");
    auto& worldState = _registry.get<WorldStateComponent>(_registry.view<WorldStateComponent>().front());
    
    worldState.generatingParams = worldState.pendingParams;
    
    _generationFuture = std::async(std::launch::async, [&, params = worldState.generatingParams]() {
        _worldGenSystem.regenerate(params);
    });
}

void ChunkManagerSystem::onSwapWorldState(const SwapWorldStateEvent& event) {
    auto& worldState = _registry.get<WorldStateComponent>(_registry.view<WorldStateComponent>().front());
    
    // The new active state is the one that was just generated
    std::swap(worldState.activeParams, worldState.generatingParams);

    // Now, force all chunks to be re-evaluated and possibly regenerated.
    std::vector<sf::Vector2i> chunksToUnload;
    for (const auto& pair : _activeChunks) {
        chunksToUnload.push_back(pair.first);
    }

    for (const auto& chunkPos : chunksToUnload) {
        unloadChunk(chunkPos);
    }
}

void ChunkManagerSystem::update(sf::Time dt) {
    if (_generationFuture.valid() && _generationFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        _generationFuture.get(); // To handle any exceptions
        _eventBus.trigger(SwapWorldStateEvent{});
    }

    const auto& camera = *_serviceLocator.camera;
    const auto& worldGrid = _registry.get<WorldGridComponent>(_registry.view<WorldGridComponent>().front());

    sf::Vector2f cameraCenter = camera.getCenter();
    sf::Vector2f viewSize = camera.getView().getSize();
    float cellSize = worldGrid.cellSize;
    const auto& chunkDims = worldGrid.chunkDimensionsInCells;

    float chunkWidthInPixels = chunkDims.x * cellSize;
    float chunkHeightInPixels = chunkDims.y * cellSize;

    int viewDistanceX = static_cast<int>(std::ceil(viewSize.x / 2.0f / chunkWidthInPixels)) + 1;
    int viewDistanceY = static_cast<int>(std::ceil(viewSize.y / 2.0f / chunkHeightInPixels)) + 1;

    sf::Vector2i centerChunk = {
        static_cast<int>(cameraCenter.x / chunkWidthInPixels),
        static_cast<int>(cameraCenter.y / chunkHeightInPixels)
    };

    std::set<sf::Vector2i, Vector2iCompare> requiredChunks;
    for (int y = centerChunk.y - viewDistanceY; y <= centerChunk.y + viewDistanceY; ++y) {
        for (int x = centerChunk.x - viewDistanceX; x <= centerChunk.x + viewDistanceX; ++x) {
            requiredChunks.insert({x, y});
        }
    }

    // Identify chunks to unload
    std::vector<sf::Vector2i> chunksToUnload;
    for (const auto& pair : _activeChunks) {
        if (requiredChunks.find(pair.first) == requiredChunks.end()) {
            chunksToUnload.push_back(pair.first);
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
    _activeChunks[chunkPos] = entity;
    LOG_INFO("ChunkManagerSystem", "Loaded chunk at (%d, %d)", chunkPos.x, chunkPos.y);
}

void ChunkManagerSystem::unloadChunk(const sf::Vector2i& chunkPos) {
    auto it = _activeChunks.find(chunkPos);
    if (it != _activeChunks.end()) {
        _registry.destroy(it->second);
        _activeChunks.erase(it);
        LOG_INFO("ChunkManagerSystem", "Unloaded chunk at (%d, %d)", chunkPos.x, chunkPos.y);
    }
}
