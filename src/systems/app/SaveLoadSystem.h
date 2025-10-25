#pragma once

#include "ecs/ISystem.h"
#include "event/EventBus.h"
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

class WorldGenerationSystem;
class ChunkManagerSystem;
class CityPlacementSystem;
class PassengerSpawnSystem;
class Camera;
struct GameState;
struct GeneratedChunkData;
struct WorldGenParams;
struct SaveGameRequestEvent;
struct LoadGameRequestEvent;

class SaveLoadSystem : public ISystem {
public:
    SaveLoadSystem(entt::registry &registry, EventBus &eventBus,
                   WorldGenerationSystem &worldGenSystem,
                   ChunkManagerSystem &chunkManagerSystem,
                   CityPlacementSystem &cityPlacementSystem,
                   PassengerSpawnSystem &passengerSpawnSystem, GameState &gameState, Camera &camera);
    ~SaveLoadSystem() override;

private:
    void onSaveGame(const SaveGameRequestEvent &event);
    void onLoadGame(const LoadGameRequestEvent &event);

    using EntityId = std::uint32_t;
    entt::entity toEntity(EntityId id, const std::unordered_map<EntityId, entt::entity> &map) const;
    EntityId toId(entt::entity entity) const;

    nlohmann::json serializeWorldGenParams(const WorldGenParams &params) const;
    WorldGenParams deserializeWorldGenParams(const nlohmann::json &data) const;
    nlohmann::json serializeWorldState() const;
    void applyWorldState(const nlohmann::json &data);
    nlohmann::json serializeChunks() const;
    std::vector<GeneratedChunkData> deserializeChunks(const nlohmann::json &data) const;
    nlohmann::json serializeEntities() const;
    std::unordered_map<EntityId, entt::entity>
    deserializeEntities(const nlohmann::json &data, std::vector<entt::entity> &lineEntities);
    nlohmann::json serializeGameState() const;
    void applyGameState(const nlohmann::json &data,
                        const std::unordered_map<EntityId, entt::entity> &map);
    nlohmann::json serializeCityPlacement() const;
    void applyCityPlacement(const nlohmann::json &data);
    nlohmann::json serializeScore() const;
    void applyScore(const nlohmann::json &data);
    nlohmann::json serializePassengerSpawn() const;
    void applyPassengerSpawn(const nlohmann::json &data);
    nlohmann::json serializeCamera() const;
    void applyCamera(const nlohmann::json &data);

    void rebuildSharedSegments(const std::vector<entt::entity> &lineEntities);
    void clearExistingEntities();

    entt::registry &_registry;
    EventBus &_eventBus;
    WorldGenerationSystem &_worldGenSystem;
    ChunkManagerSystem &_chunkManagerSystem;
    CityPlacementSystem &_cityPlacementSystem;
    PassengerSpawnSystem &_passengerSpawnSystem;
    GameState &_gameState;
    Camera &_camera;
    entt::scoped_connection _saveConnection;
    entt::scoped_connection _loadConnection;
};
