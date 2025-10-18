#pragma once

#include "WorldGenerationSystem.h"
#include "ecs/ISystem.h"
#include "event/EventBus.h"
#include "event/InputEvents.h"
#include "world/WorldData.h"
#include <SFML/System/Vector2.hpp>
#include <condition_variable>
#include <future>
#include <map>
#include <mutex>
#include <queue>
#include <vector>

class Camera;
class ThreadPool;

struct Vector2iCompare {
    bool operator()(const sf::Vector2i &a, const sf::Vector2i &b) const {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y;
    }
};

class ChunkManagerSystem : public ISystem, public IUpdatable {
public:
    explicit ChunkManagerSystem(entt::registry& registry, EventBus& eventBus, WorldGenerationSystem& worldGenSystem, Camera& camera, ThreadPool& threadPool);
    ~ChunkManagerSystem();
    void update(sf::Time dt) override;

private:
    // Event Handlers
    void onRegenerateWorld(const RegenerateWorldRequestEvent &event);
    void onSwapWorldState(const SwapWorldStateEvent &event);
    void onImmediateRedraw(const ImmediateRedrawEvent &event);

    // Chunk Management
    void loadChunk(const sf::Vector2i &chunkPos);
    void unloadChunk(const sf::Vector2i &chunkPos);
    void processCompletedChunks();
    void processChunkRegeneration();

    // Update Helpers
    void handleWorldGeneration();
    void handleChunkLoading();
    void updateActiveChunks();
    bool requiresFullReload(const WorldGenParams &currentParams,
                            const WorldGenParams &newParams) const;
    void startSmoothRegeneration(const WorldGenParams &params);

    struct PendingChunkUpdate {
        sf::Vector2i chunkGridPosition;
        entt::entity entity;
        std::future<GeneratedChunkData> future;
        std::size_t generationId;
    };

    // Member Variables
    entt::registry& _registry;
    EventBus& _eventBus;
    WorldGenerationSystem& _worldGenSystem;
    Camera& _camera;
    ThreadPool& _threadPool;

    std::map<sf::Vector2i, entt::entity, Vector2iCompare> _activeChunks;
    std::set<sf::Vector2i, Vector2iCompare> _chunksBeingLoaded;
    std::vector<std::future<GeneratedChunkData>> _chunkLoadFutures;

    std::mutex _completedChunksMutex;
    std::queue<GeneratedChunkData> _completedChunks;

    std::future<void> _generationFuture;
    std::vector<PendingChunkUpdate> _pendingChunkUpdates;
    std::size_t _currentGenerationId = 0;
    bool _performingFullReload = false;
    bool _smoothRegenPending = false;

    entt::scoped_connection _regenerateWorldListener;
    entt::scoped_connection _swapWorldStateListener;
    entt::scoped_connection _immediateRedrawListener;
};
