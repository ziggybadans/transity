#pragma once

#include "WorldGenerationSystem.h"
#include "core/ServiceLocator.h"
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

struct Vector2iCompare {
    bool operator()(const sf::Vector2i &a, const sf::Vector2i &b) const {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y;
    }
};

class ChunkManagerSystem : public ISystem, public IUpdatable {
public:
    explicit ChunkManagerSystem(ServiceLocator &serviceLocator,
                                WorldGenerationSystem &worldGenSystem, EventBus &eventBus);
    ~ChunkManagerSystem();
    void update(sf::Time dt) override;

private:
    void onRegenerateWorld(const RegenerateWorldRequestEvent &event);
    void loadChunk(const sf::Vector2i &chunkPos);
    void unloadChunk(const sf::Vector2i &chunkPos);
    void processLoadingQueue();
    void processCompletedChunks();

    void onImmediateRedraw(const ImmediateRedrawEvent &event);
    entt::connection _immediateRedrawListener;

    ServiceLocator &_serviceLocator;
    WorldGenerationSystem &_worldGenSystem;
    entt::registry &_registry;
    EventBus &_eventBus;

    std::map<sf::Vector2i, entt::entity, Vector2iCompare> _activeChunks;
    std::set<sf::Vector2i, Vector2iCompare> _chunksBeingLoaded;
    std::vector<std::future<GeneratedChunkData>> _chunkLoadFutures;

    std::mutex _completedChunksMutex;
    std::queue<GeneratedChunkData> _completedChunks;

    entt::connection _regenerateWorldListener;
    int _viewDistance = 4;

    void onSwapWorldState(const SwapWorldStateEvent &event);
    entt::connection _swapWorldStateListener;
    std::future<void> _generationFuture;
};