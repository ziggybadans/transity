#pragma once

#include "../core/ISystem.h"
#include "../core/ServiceLocator.h"
#include "WorldGenerationSystem.h"
#include "../event/EventBus.h"
#include "../event/InputEvents.h"
#include <map>
#include <future>
#include <SFML/System/Vector2.hpp>

struct Vector2iCompare {
    bool operator()(const sf::Vector2i& a, const sf::Vector2i& b) const {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y;
    }
};

class ChunkManagerSystem : public ISystem {
public:
    explicit ChunkManagerSystem(ServiceLocator& serviceLocator, WorldGenerationSystem& worldGenSystem, EventBus& eventBus);
    ~ChunkManagerSystem();
    void update(sf::Time dt) override;

private:
    void onRegenerateWorld(const RegenerateWorldRequestEvent& event);
    void loadChunk(const sf::Vector2i& chunkPos);
    void unloadChunk(const sf::Vector2i& chunkPos);

    ServiceLocator& _serviceLocator;
    WorldGenerationSystem& _worldGenSystem;
    entt::registry& _registry;
    EventBus& _eventBus;
    
    std::map<sf::Vector2i, entt::entity, Vector2iCompare> _activeChunks;

    entt::connection _regenerateWorldListener;
    int _viewDistance = 4; // In chunks

    void onSwapWorldState(const SwapWorldStateEvent& event);
    entt::connection _swapWorldStateListener;
    std::future<void> _generationFuture;
};