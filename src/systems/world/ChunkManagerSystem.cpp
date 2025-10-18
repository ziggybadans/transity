#include "ChunkManagerSystem.h"
#include "Logger.h"
#include "components/RenderComponents.h"
#include "components/WorldComponents.h"
#include "render/Camera.h"
#include "core/ThreadPool.h"
#include <algorithm>
#include <chrono>
#include <cmath>

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
    auto &worldState = _registry.emplace<WorldStateComponent>(entity);
    worldState.activeParams = _worldGenSystem.getParams();
    worldState.generatingParams = worldState.activeParams;
    worldState.pendingParams = worldState.activeParams;
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
        LOG_WARN("ChunkManagerSystem", "Regeneration requested while a previous generation is still running.");
        return;
    }

    WorldGenParams params = *event.params;

    LOG_INFO("ChunkManagerSystem",
             "Starting world regeneration for %d x %d chunks.",
             params.worldDimensionsInChunks.x, params.worldDimensionsInChunks.y);
    auto &worldState =
        _registry.get<WorldStateComponent>(_registry.view<WorldStateComponent>().front());

    const bool hasActiveChunks = !_activeChunks.empty();
    const bool needsFullReload = !hasActiveChunks
                                 || requiresFullReload(worldState.activeParams, params);

    worldState.generatingParams = params;
    worldState.pendingParams = params;

    if (needsFullReload) {
        _performingFullReload = true;
        _smoothRegenPending = false;
        _generationFuture = std::async(std::launch::async, [&, params]() { _worldGenSystem.regenerate(params); });
        return;
    }

    LOG_DEBUG("ChunkManagerSystem", "Applying smooth regeneration (params update without structural changes).");

    if (!_pendingChunkUpdates.empty()) {
        _smoothRegenPending = true;
        LOG_DEBUG("ChunkManagerSystem", "Smooth regeneration already running. Queuing new parameters.");
        return;
    }

    worldState.activeParams = params;
    _worldGenSystem.regenerate(params);
    startSmoothRegeneration(params);
    _smoothRegenPending = false;
}

void ChunkManagerSystem::onSwapWorldState(const SwapWorldStateEvent &event) {
    if (!_performingFullReload) {
        return;
    }

    auto &worldState =
        _registry.get<WorldStateComponent>(_registry.view<WorldStateComponent>().front());

    std::swap(worldState.activeParams, worldState.generatingParams);
    worldState.pendingParams = worldState.activeParams;

    std::vector<sf::Vector2i> chunksToUnload;
    for (const auto &pair : _activeChunks) {
        chunksToUnload.push_back(pair.first);
    }

    for (const auto &chunkPos : chunksToUnload) {
        unloadChunk(chunkPos);
    }

    _pendingChunkUpdates.clear();
    ++_currentGenerationId;
    _performingFullReload = false;
    _smoothRegenPending = false;
}

void ChunkManagerSystem::update(sf::Time dt) {
    handleWorldGeneration();
    handleChunkLoading();
    processChunkRegeneration();
    if (_smoothRegenPending && _pendingChunkUpdates.empty() && !_performingFullReload) {
        auto &worldState =
            _registry.get<WorldStateComponent>(_registry.view<WorldStateComponent>().front());
        LOG_DEBUG("ChunkManagerSystem", "Processing queued smooth regeneration request.");
        WorldGenParams params = worldState.pendingParams;
        worldState.activeParams = params;
        _worldGenSystem.regenerate(params);
        startSmoothRegeneration(params);
        _smoothRegenPending = false;
    }
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

void ChunkManagerSystem::processChunkRegeneration() {
    if (_pendingChunkUpdates.empty()) {
        return;
    }

    const std::size_t currentGenerationId = _currentGenerationId;

    _pendingChunkUpdates.erase(
        std::remove_if(_pendingChunkUpdates.begin(), _pendingChunkUpdates.end(),
                       [this, currentGenerationId](PendingChunkUpdate &pending) {
                           if (pending.future.wait_for(std::chrono::seconds(0))
                               != std::future_status::ready) {
                               return false;
                           }

                           GeneratedChunkData chunkData = pending.future.get();
                           if (pending.generationId != currentGenerationId) {
                               return true;
                           }

                           if (!_registry.valid(pending.entity)
                               || !_registry.all_of<ChunkTerrainComponent, ChunkStateComponent>(
                                   pending.entity)) {
                               return true;
                           }

                           auto &chunkTerrain =
                               _registry.get<ChunkTerrainComponent>(pending.entity);
                           chunkTerrain.cells = std::move(chunkData.cells);

                           if (_registry.all_of<ChunkNoiseComponent>(pending.entity)) {
                               auto &noise =
                                   _registry.get<ChunkNoiseComponent>(pending.entity);
                               noise.noiseValues = std::move(chunkData.noiseValues);
                               noise.rawNoiseValues = std::move(chunkData.rawNoiseValues);
                           }

                           auto &chunkState = _registry.get<ChunkStateComponent>(pending.entity);
                           chunkState.isMeshDirty = true;
                           return true;
                       }),
        _pendingChunkUpdates.end());
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

bool ChunkManagerSystem::requiresFullReload(const WorldGenParams &currentParams,
                                            const WorldGenParams &newParams) const {
    return currentParams.worldDimensionsInChunks != newParams.worldDimensionsInChunks
           || currentParams.chunkDimensionsInCells != newParams.chunkDimensionsInCells
           || currentParams.cellSize != newParams.cellSize;
}

void ChunkManagerSystem::startSmoothRegeneration(const WorldGenParams &params) {
    if (_activeChunks.empty()) {
        return;
    }

    ++_currentGenerationId;
    const std::size_t generationId = _currentGenerationId;

    struct ChunkRegenTarget {
        sf::Vector2i position;
        entt::entity entity;
    };

    std::vector<ChunkRegenTarget> targets;
    targets.reserve(_activeChunks.size());

    for (const auto &pair : _activeChunks) {
        if (_registry.valid(pair.second)) {
            targets.push_back({pair.first, pair.second});
        }
    }

    if (targets.empty()) {
        return;
    }

    const float chunkWidth = params.chunkDimensionsInCells.x * params.cellSize;
    const float chunkHeight = params.chunkDimensionsInCells.y * params.cellSize;
    const sf::Vector2f cameraCenter = _camera.getCenter();

    auto distanceSquared = [&](const sf::Vector2i &chunkPos) {
        const float chunkCenterX =
            (static_cast<float>(chunkPos.x) + 0.5f) * chunkWidth;
        const float chunkCenterY =
            (static_cast<float>(chunkPos.y) + 0.5f) * chunkHeight;
        const float dx = chunkCenterX - cameraCenter.x;
        const float dy = chunkCenterY - cameraCenter.y;
        return dx * dx + dy * dy;
    };

    std::sort(targets.begin(), targets.end(),
              [&](const ChunkRegenTarget &lhs, const ChunkRegenTarget &rhs) {
                  return distanceSquared(lhs.position) < distanceSquared(rhs.position);
              });

    for (auto &target : targets) {
        auto future = _threadPool.enqueue([this, chunkPos = target.position]() {
            return _worldGenSystem.generateChunkData(chunkPos);
        });

        _pendingChunkUpdates.push_back(
            PendingChunkUpdate{target.position, target.entity, std::move(future), generationId});
    }
}
