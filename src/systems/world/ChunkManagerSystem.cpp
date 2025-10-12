#include "ChunkManagerSystem.h"
#include "Logger.h"
#include "components/RenderComponents.h"
#include "components/WorldComponents.h"
#include "render/Camera.h"
#include "core/ThreadPool.h"

ChunkManagerSystem::ChunkManagerSystem(entt::registry& registry, EventBus& eventBus, WorldGenerationSystem& worldGenSystem, Camera& camera, ThreadPool& threadPool)
    : _registry(registry), _eventBus(eventBus), _worldGenSystem(worldGenSystem), _camera(camera), _threadPool(threadPool), _activeChunks() {
    _regenerateWorldListener = _eventBus.sink<RegenerateWorldRequestEvent>()
                                   .connect<&ChunkManagerSystem::onRegenerateWorld>(this);
    _swapWorldStateListener =
        _eventBus.sink<SwapWorldStateEvent>().connect<&ChunkManagerSystem::onSwapWorldState>(this);
    _immediateRedrawListener =
        _eventBus.sink<ImmediateRedrawEvent>().connect<&ChunkManagerSystem::onImmediateRedraw>(
            this);

    auto entity = _registry.create();
    _registry.emplace<WorldStateComponent>(entity);
}

ChunkManagerSystem::~ChunkManagerSystem() {
    _eventBus.sink<RegenerateWorldRequestEvent>().disconnect(this);
    _eventBus.sink<SwapWorldStateEvent>().disconnect(this);
    _eventBus.sink<ImmediateRedrawEvent>().disconnect(this);
}

void ChunkManagerSystem::onImmediateRedraw(const ImmediateRedrawEvent &event) {
    for (auto const &[pos, entity] : _activeChunks) {
        if (_registry.valid(entity)) {
            auto &chunk = _registry.get<ChunkStateComponent>(entity);
            chunk.isMeshDirty = true;
        }
    }
}

void ChunkManagerSystem::onRegenerateWorld(const RegenerateWorldRequestEvent &event) {
    if (_generationFuture.valid()) {
        LOG_WARN("ChunkManager", "Already generating world, regenerate request ignored.");
        return;
    }

    const WorldGenParams &params = *event.params;

    LOG_DEBUG("ChunkManagerSystem", "Regenerating world.");
    auto &worldState =
        _registry.get<WorldStateComponent>(_registry.view<WorldStateComponent>().front());

    worldState.generatingParams = *event.params;

    _generationFuture = std::async(std::launch::async, [&, params = worldState.generatingParams]() {
        _worldGenSystem.regenerate(params);
    });
}

void ChunkManagerSystem::onSwapWorldState(const SwapWorldStateEvent &event) {
    auto &worldState =
        _registry.get<WorldStateComponent>(_registry.view<WorldStateComponent>().front());

    std::swap(worldState.activeParams, worldState.generatingParams);

    std::vector<sf::Vector2i> chunksToUnload;
    for (const auto &pair : _activeChunks) {
        chunksToUnload.push_back(pair.first);
    }

    for (const auto &chunkPos : chunksToUnload) {
        unloadChunk(chunkPos);
    }
}

void ChunkManagerSystem::update(sf::Time dt) {
    handleWorldGeneration();
    handleChunkLoading();
    updateChunkLODs();
    updateActiveChunks();
}

void ChunkManagerSystem::loadChunk(const sf::Vector2i &chunkPos) {
    const auto &worldParams = _worldGenSystem.getParams();
    const auto &worldDims = worldParams.worldDimensionsInChunks;

    if (chunkPos.x < 0 || chunkPos.x >= worldDims.x || chunkPos.y < 0
        || chunkPos.y >= worldDims.y) {
        return;
    }

    _chunksBeingLoaded.insert(chunkPos);

    auto task = [this, chunkPos]() { return _worldGenSystem.generateChunkData(chunkPos); };
    _chunkLoadFutures.emplace_back(_threadPool.enqueue(task));
}

void ChunkManagerSystem::unloadChunk(const sf::Vector2i &chunkPos) {
    auto it = _activeChunks.find(chunkPos);
    if (it != _activeChunks.end()) {
        _registry.destroy(it->second);
        _activeChunks.erase(it);
        LOG_TRACE("ChunkManagerSystem", "Unloaded chunk at (%d, %d)", chunkPos.x, chunkPos.y);
    }
}

void ChunkManagerSystem::processCompletedChunks() {
    std::lock_guard<std::mutex> lock(_completedChunksMutex);
    while (!_completedChunks.empty()) {
        GeneratedChunkData chunkData = std::move(_completedChunks.front());
        _completedChunks.pop();

        sf::Vector2i chunkPos = chunkData.chunkGridPosition;

        auto entity = _registry.create();
        _registry.emplace<ChunkPositionComponent>(entity, chunkPos);
        _registry.emplace<ChunkTerrainComponent>(entity, std::move(chunkData.cells));
        _registry.emplace<ChunkNoiseComponent>(entity, std::move(chunkData.noiseValues),
                                               std::move(chunkData.rawNoiseValues));
        _registry.emplace<ChunkStateComponent>(entity);
        _registry.emplace<ChunkMeshComponent>(entity);

        _activeChunks[chunkPos] = entity;
        _chunksBeingLoaded.erase(chunkPos);
        LOG_TRACE("ChunkManagerSystem", "Finalized loaded chunk at (%d, %d)", chunkPos.x,
                 chunkPos.y);
    }
}

void ChunkManagerSystem::handleWorldGeneration() {
    if (_generationFuture.valid()
        && _generationFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        _generationFuture.get();
        _eventBus.trigger(SwapWorldStateEvent{});
    }
}

void ChunkManagerSystem::handleChunkLoading() {
    _chunkLoadFutures.erase(
        std::remove_if(_chunkLoadFutures.begin(), _chunkLoadFutures.end(),
                       [this](std::future<GeneratedChunkData> &f) {
                           if (f.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                               std::lock_guard<std::mutex> lock(_completedChunksMutex);
                               _completedChunks.push(f.get());
                               return true;
                           }
                           return false;
                       }),
        _chunkLoadFutures.end());

    processCompletedChunks();
}

void ChunkManagerSystem::updateChunkLODs() {
    float zoom = _camera.getZoom();
    LODLevel currentLOD = LODLevel::LOD0;
    if (zoom < 0.15f) {
        currentLOD = LODLevel::LOD3;
    } else if (zoom < 0.4f) {
        currentLOD = LODLevel::LOD2;
    } else if (zoom < 0.8f) {
        currentLOD = LODLevel::LOD1;
    }

    for (auto entity : _registry.view<ChunkStateComponent>()) {
        auto &chunkState = _registry.get<ChunkStateComponent>(entity);
        if (chunkState.lodLevel != currentLOD) {
            chunkState.lodLevel = currentLOD;
        }
    }
}

void ChunkManagerSystem::updateActiveChunks() {
    const auto& worldParams = _worldGenSystem.getParams();
    float chunkWidthInPixels = worldParams.chunkDimensionsInCells.x * worldParams.cellSize;
    float chunkHeightInPixels = worldParams.chunkDimensionsInCells.y * worldParams.cellSize;

    sf::Vector2f cameraCenter = _camera.getCenter();
    sf::Vector2f viewSize = _camera.getView().getSize();

    int viewDistanceX = static_cast<int>(std::ceil(viewSize.x / 2.0f / chunkWidthInPixels)) + 1;
    int viewDistanceY = static_cast<int>(std::ceil(viewSize.y / 2.0f / chunkHeightInPixels)) + 1;

    sf::Vector2i centerChunk = {static_cast<int>(cameraCenter.x / chunkWidthInPixels),
                                static_cast<int>(cameraCenter.y / chunkHeightInPixels)};

    std::set<sf::Vector2i, Vector2iCompare> requiredChunks;
    for (int y = centerChunk.y - viewDistanceY; y <= centerChunk.y + viewDistanceY; ++y) {
        for (int x = centerChunk.x - viewDistanceX; x <= centerChunk.x + viewDistanceX; ++x) {
            requiredChunks.insert({x, y});
        }
    }

    std::vector<sf::Vector2i> chunksToUnload;
    for (const auto &pair : _activeChunks) {
        if (requiredChunks.find(pair.first) == requiredChunks.end()) {
            chunksToUnload.push_back(pair.first);
        }
    }

    for (const auto &chunkPos : chunksToUnload) {
        unloadChunk(chunkPos);
    }

    for (const auto &chunkPos : requiredChunks) {
        if (_activeChunks.find(chunkPos) == _activeChunks.end()
            && _chunksBeingLoaded.find(chunkPos) == _chunksBeingLoaded.end()) {
            loadChunk(chunkPos);
        }
    }
}