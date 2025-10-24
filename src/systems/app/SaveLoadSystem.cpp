#include "SaveLoadSystem.h"
#include "Constants.h"
#include "Logger.h"
#include "app/GameState.h"
#include "app/InteractionMode.h"
#include "components/GameLogicComponents.h"
#include "components/LineComponents.h"
#include "components/PassengerComponents.h"
#include "components/RenderComponents.h"
#include "components/TrainComponents.h"
#include "components/WorldComponents.h"
#include "event/LineEvents.h"
#include "event/UIEvents.h"
#include "render/Camera.h"
#include "systems/gameplay/CityPlacementSystem.h"
#include "systems/gameplay/PassengerSpawnSystem.h"
#include "systems/world/ChunkManagerSystem.h"
#include "systems/world/WorldGenerationSystem.h"
#include "world/WorldData.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <filesystem>
#include <fstream>
#include <optional>
#include <queue>
#include <sstream>
#include <utility>

namespace {
constexpr int SaveFormatVersion = 1;

struct PendingEntity {
    entt::entity entity;
    nlohmann::json components;
};

sf::Vector2f deserializeVec2(const nlohmann::json &data) {
    return {data.value("x", 0.0f), data.value("y", 0.0f)};
}

sf::Color deserializeColor(const nlohmann::json &data) {
    if (data.is_array() && data.size() == 4) {
        return sf::Color(data[0].get<int>(), data[1].get<int>(), data[2].get<int>(),
                         data[3].get<int>());
    }
    return sf::Color::White;
}

TerrainType terrainFromInt(int value) {
    switch (value) {
    case 0:
        return TerrainType::WATER;
    case 1:
        return TerrainType::LAND;
    case 2:
        return TerrainType::RIVER;
    default:
        return TerrainType::WATER;
    }
}

CityType cityTypeFromInt(int value) {
    switch (value) {
    case 0:
        return CityType::CAPITAL;
    case 1:
        return CityType::TOWN;
    case 2:
        return CityType::SUBURB;
    default:
        return CityType::TOWN;
    }
}

PassengerState passengerStateFromInt(int value) {
    switch (value) {
    case 0:
        return PassengerState::WAITING_FOR_TRAIN;
    case 1:
        return PassengerState::ON_TRAIN;
    case 2:
        return PassengerState::ARRIVED;
    default:
        return PassengerState::WAITING_FOR_TRAIN;
    }
}

TrainState trainStateFromInt(int value) {
    switch (value) {
    case 0:
        return TrainState::STOPPED;
    case 1:
        return TrainState::ACCELERATING;
    case 2:
        return TrainState::MOVING;
    case 3:
        return TrainState::DECELERATING;
    default:
        return TrainState::STOPPED;
    }
}

TrainDirection trainDirectionFromInt(int value) {
    return value == 1 ? TrainDirection::BACKWARD : TrainDirection::FORWARD;
}

LinePointType linePointTypeFromInt(int value) {
    return value == 0 ? LinePointType::STOP : LinePointType::CONTROL_POINT;
}

std::optional<size_t> deserializeOptionalIndex(const nlohmann::json &data, const char *key) {
    if (data.contains(key) && !data[key].is_null()) {
        return data[key].get<size_t>();
    }
    return std::nullopt;
}

InteractionMode interactionModeFromInt(int value) {
    switch (value) {
    case 0:
        return InteractionMode::SELECT;
    case 1:
        return InteractionMode::CREATE_LINE;
    case 2:
        return InteractionMode::EDIT_LINE;
    case 3:
        return InteractionMode::CREATE_PASSENGER;
    default:
        return InteractionMode::SELECT;
    }
}

AppState appStateFromInt(int value) {
    switch (value) {
    case 0:
        return AppState::LOADING;
    case 1:
        return AppState::PLAYING;
    case 2:
        return AppState::QUITTING;
    default:
        return AppState::PLAYING;
    }
}
} // namespace

struct ActiveLine;
struct LinePreview;

SaveLoadSystem::SaveLoadSystem(entt::registry &registry, EventBus &eventBus,
                               WorldGenerationSystem &worldGenSystem,
                               ChunkManagerSystem &chunkManagerSystem,
                               CityPlacementSystem &cityPlacementSystem,
                               PassengerSpawnSystem &passengerSpawnSystem, GameState &gameState,
                               Camera &camera)
    : _registry(registry), _eventBus(eventBus), _worldGenSystem(worldGenSystem),
      _chunkManagerSystem(chunkManagerSystem), _cityPlacementSystem(cityPlacementSystem),
      _passengerSpawnSystem(passengerSpawnSystem), _gameState(gameState), _camera(camera) {
    _saveConnection =
        _eventBus.sink<SaveGameRequestEvent>().connect<&SaveLoadSystem::onSaveGame>(this);
    _loadConnection =
        _eventBus.sink<LoadGameRequestEvent>().connect<&SaveLoadSystem::onLoadGame>(this);
}

SaveLoadSystem::~SaveLoadSystem() {
    _saveConnection.release();
    _loadConnection.release();
}

void SaveLoadSystem::onSaveGame(const SaveGameRequestEvent &event) {
    try {
        std::filesystem::path path(event.path);
        if (path.has_parent_path() && !path.parent_path().empty()) {
            std::filesystem::create_directories(path.parent_path());
        }

        nlohmann::json root;
        root["version"] = SaveFormatVersion;
        root["world_generation"] = serializeWorldGenParams(_worldGenSystem.getParams());
        root["world_state"] = serializeWorldState();
        root["chunks"] = serializeChunks();
        root["entities"] = serializeEntities();
        root["game_state"] = serializeGameState();
        root["city_placement"] = serializeCityPlacement();
        root["score"] = serializeScore();
        root["passenger_spawn"] = serializePassengerSpawn();
        root["camera"] = serializeCamera();

        std::ofstream out(path);
        if (!out) {
            LOG_ERROR("SaveLoadSystem", "Failed to open file for saving: %s", event.path.c_str());
            return;
        }
        out << root.dump(2);
        LOG_INFO("SaveLoadSystem", "Game saved to %s", event.path.c_str());
    } catch (const std::exception &ex) {
        LOG_ERROR("SaveLoadSystem", "Save failed: %s", ex.what());
    }
}

void SaveLoadSystem::onLoadGame(const LoadGameRequestEvent &event) {
    try {
        std::filesystem::path path(event.path);
        if (!std::filesystem::exists(path)) {
            LOG_ERROR("SaveLoadSystem", "Save file does not exist: %s", event.path.c_str());
            return;
        }

        std::ifstream in(path);
        if (!in) {
            LOG_ERROR("SaveLoadSystem", "Failed to open save file: %s", event.path.c_str());
            return;
        }

        nlohmann::json root;
        in >> root;

        if (root.value("version", 0) != SaveFormatVersion) {
            LOG_ERROR("SaveLoadSystem", "Unsupported save version in %s", event.path.c_str());
            return;
        }

        clearExistingEntities();

        if (root.contains("world_generation")) {
            auto params = deserializeWorldGenParams(root["world_generation"]);
            _worldGenSystem.setParams(params);
        }

        if (root.contains("world_state")) {
            applyWorldState(root["world_state"]);
        } else if (root.contains("world_generation")) {
            nlohmann::json fallback;
            fallback["active"] = root["world_generation"];
            fallback["pending"] = root["world_generation"];
            fallback["generating"] = root["world_generation"];
            applyWorldState(fallback);
        }

        if (root.contains("chunks")) {
            auto chunkData = deserializeChunks(root["chunks"]);
            _chunkManagerSystem.loadChunksFromData(chunkData);
        }

        std::vector<entt::entity> lineEntities;
        std::unordered_map<EntityId, entt::entity> entityMap;
        if (root.contains("entities")) {
            entityMap = deserializeEntities(root["entities"], lineEntities);
        }

        if (root.contains("game_state")) {
            applyGameState(root["game_state"], entityMap);
        } else {
            _gameState.selectedEntity.reset();
            _gameState.passengerOriginStation.reset();
            _gameState.currentAppState = AppState::PLAYING;
        }

        if (root.contains("city_placement")) {
            applyCityPlacement(root["city_placement"]);
        }

        if (root.contains("score")) {
            applyScore(root["score"]);
        }

        if (root.contains("passenger_spawn")) {
            applyPassengerSpawn(root["passenger_spawn"]);
        }

        if (root.contains("camera")) {
            applyCamera(root["camera"]);
        }

        rebuildSharedSegments(lineEntities);
        LOG_INFO("SaveLoadSystem", "Game loaded from %s", event.path.c_str());
    } catch (const std::exception &ex) {
        LOG_ERROR("SaveLoadSystem", "Load failed: %s", ex.what());
    }
}

entt::entity SaveLoadSystem::toEntity(EntityId id,
                                      const std::unordered_map<EntityId, entt::entity> &map) const {
    if (id == 0) return entt::null;
    if (auto it = map.find(id); it != map.end()) {
        return it->second;
    }
    return entt::null;
}

SaveLoadSystem::EntityId SaveLoadSystem::toId(entt::entity entity) const {
    return entity == entt::null ? 0 : static_cast<EntityId>(entt::to_integral(entity));
}

nlohmann::json SaveLoadSystem::serializeWorldGenParams(const WorldGenParams &params) const {
    nlohmann::json result;
    nlohmann::json layers = nlohmann::json::array();
    for (const auto &layer : params.noiseLayers) {
        nlohmann::json l;
        l["name"] = layer.name;
        l["seed"] = layer.seed;
        l["frequency"] = layer.frequency;
        l["noise_type"] = static_cast<int>(layer.noiseType);
        l["fractal_type"] = static_cast<int>(layer.fractalType);
        l["octaves"] = layer.octaves;
        l["lacunarity"] = layer.lacunarity;
        l["gain"] = layer.gain;
        l["weight"] = layer.weight;
        layers.push_back(l);
    }
    result["noise_layers"] = layers;
    result["land_threshold"] = params.landThreshold;
    result["coastline_distortion_strength"] = params.coastlineDistortionStrength;

    nlohmann::json shape = nlohmann::json::array();
    for (const auto &point : params.continentShape) {
        shape.push_back({{"x", point.x}, {"y", point.y}});
    }
    result["continent_shape"] = shape;

    result["elevation"] = {{"max_elevation", params.elevation.maxElevation},
                           {"exponent", params.elevation.elevationExponent}};

    result["world_dimensions_in_chunks"] = {{"x", params.worldDimensionsInChunks.x},
                                            {"y", params.worldDimensionsInChunks.y}};
    result["chunk_dimensions_in_cells"] = {{"x", params.chunkDimensionsInCells.x},
                                           {"y", params.chunkDimensionsInCells.y}};
    result["cell_size"] = params.cellSize;
    return result;
}

WorldGenParams SaveLoadSystem::deserializeWorldGenParams(const nlohmann::json &data) const {
    WorldGenParams params;
    if (data.contains("noise_layers")) {
        params.noiseLayers.clear();
        for (const auto &layerData : data["noise_layers"]) {
            NoiseLayer layer;
            layer.name = layerData.value("name", std::string("Layer"));
            layer.seed = layerData.value("seed", 1337);
            layer.frequency = layerData.value("frequency", 0.02f);
            layer.noiseType = static_cast<FastNoiseLite::NoiseType>(
                layerData.value("noise_type", static_cast<int>(FastNoiseLite::NoiseType_Perlin)));
            layer.fractalType = static_cast<FastNoiseLite::FractalType>(
                layerData.value("fractal_type",
                                static_cast<int>(FastNoiseLite::FractalType_FBm)));
            layer.octaves = layerData.value("octaves", 5);
            layer.lacunarity = layerData.value("lacunarity", 2.0f);
            layer.gain = layerData.value("gain", 0.5f);
            layer.weight = layerData.value("weight", 1.0f);
            params.noiseLayers.push_back(layer);
        }
    }

    params.landThreshold = data.value("land_threshold", 0.35f);
    params.coastlineDistortionStrength = data.value("coastline_distortion_strength", 0.0f);

    params.continentShape.clear();
    if (data.contains("continent_shape")) {
        for (const auto &point : data["continent_shape"]) {
            params.continentShape.push_back({point.value("x", 0.0f), point.value("y", 0.0f)});
        }
    }

    if (data.contains("elevation")) {
        const auto &elev = data["elevation"];
        params.elevation.maxElevation = elev.value("max_elevation", 200.0f);
        params.elevation.elevationExponent = elev.value("exponent", 1.0f);
    }

    if (data.contains("world_dimensions_in_chunks")) {
        params.worldDimensionsInChunks.x =
            data["world_dimensions_in_chunks"].value("x", params.worldDimensionsInChunks.x);
        params.worldDimensionsInChunks.y =
            data["world_dimensions_in_chunks"].value("y", params.worldDimensionsInChunks.y);
    }
    if (data.contains("chunk_dimensions_in_cells")) {
        params.chunkDimensionsInCells.x =
            data["chunk_dimensions_in_cells"].value("x", params.chunkDimensionsInCells.x);
        params.chunkDimensionsInCells.y =
            data["chunk_dimensions_in_cells"].value("y", params.chunkDimensionsInCells.y);
    }
    params.cellSize = data.value("cell_size", params.cellSize);
    return params;
}

nlohmann::json SaveLoadSystem::serializeWorldState() const {
    nlohmann::json data;
    auto view = _registry.view<WorldStateComponent>();
    for (auto entity : view) {
        const auto &worldState = view.get<WorldStateComponent>(entity);
        data["active"] = serializeWorldGenParams(worldState.activeParams);
        data["pending"] = serializeWorldGenParams(worldState.pendingParams);
        data["generating"] = serializeWorldGenParams(worldState.generatingParams);
        break;
    }
    return data;
}

void SaveLoadSystem::applyWorldState(const nlohmann::json &data) {
    auto view = _registry.view<WorldStateComponent>();
    for (auto entity : view) {
        auto &worldState = view.get<WorldStateComponent>(entity);
        if (data.contains("active")) {
            worldState.activeParams = deserializeWorldGenParams(data["active"]);
        }
        if (data.contains("pending")) {
            worldState.pendingParams = deserializeWorldGenParams(data["pending"]);
        } else {
            worldState.pendingParams = worldState.activeParams;
        }
        if (data.contains("generating")) {
            worldState.generatingParams = deserializeWorldGenParams(data["generating"]);
        } else {
            worldState.generatingParams = worldState.activeParams;
        }
        _worldGenSystem.setParams(worldState.activeParams);
        break;
    }
}

nlohmann::json SaveLoadSystem::serializeChunks() const {
    nlohmann::json chunks = nlohmann::json::array();
    auto chunkView = _registry.view<const ChunkPositionComponent, const ChunkTerrainComponent,
                                    const ChunkElevationComponent>();
    for (auto entity : chunkView) {
        const auto &pos = chunkView.get<const ChunkPositionComponent>(entity);
        const auto &terrain = chunkView.get<const ChunkTerrainComponent>(entity);
        const auto &elevation = chunkView.get<const ChunkElevationComponent>(entity);

        nlohmann::json chunk;
        chunk["x"] = pos.chunkGridPosition.x;
        chunk["y"] = pos.chunkGridPosition.y;

        nlohmann::json cells = nlohmann::json::array();
        for (TerrainType type : terrain.cells) {
            cells.push_back(static_cast<int>(type));
        }
        chunk["cells"] = cells;

        nlohmann::json elevations = nlohmann::json::array();
        for (float value : elevation.elevations) {
            elevations.push_back(value);
        }
        chunk["elevations"] = elevations;

        chunks.push_back(chunk);
    }
    return chunks;
}

std::vector<GeneratedChunkData> SaveLoadSystem::deserializeChunks(
    const nlohmann::json &data) const {
    std::vector<GeneratedChunkData> chunks;
    if (!data.is_array()) {
        return chunks;
    }

    chunks.reserve(data.size());
    for (const auto &chunkData : data) {
        GeneratedChunkData chunk;
        chunk.chunkGridPosition.x = chunkData.value("x", 0);
        chunk.chunkGridPosition.y = chunkData.value("y", 0);
        if (chunkData.contains("cells")) {
            for (const auto &cell : chunkData["cells"]) {
                chunk.cells.push_back(terrainFromInt(cell.get<int>()));
            }
        }
        if (chunkData.contains("elevations")) {
            for (const auto &elev : chunkData["elevations"]) {
                chunk.elevations.push_back(elev.get<float>());
            }
        }
        chunks.push_back(std::move(chunk));
    }
    return chunks;
}

nlohmann::json SaveLoadSystem::serializeEntities() const {
    nlohmann::json entities = nlohmann::json::array();
    auto &entityStorage = _registry.storage<entt::entity>();
    for (auto [entity] : entityStorage.each()) {
        if (_registry.any_of<ChunkPositionComponent>(entity)) continue;
        if (_registry.any_of<GameScoreComponent>(entity)) continue;
        if (_registry.any_of<WorldStateComponent>(entity)) continue;

        nlohmann::json comps;

        if (auto *position = _registry.try_get<PositionComponent>(entity)) {
            comps["PositionComponent"] = {{"x", position->coordinates.x},
                                          {"y", position->coordinates.y}};
        }

        if (auto *name = _registry.try_get<NameComponent>(entity)) {
            comps["NameComponent"] = {{"name", name->name}};
        }

        if (auto *city = _registry.try_get<CityComponent>(entity)) {
            nlohmann::json jsonLines = nlohmann::json::array();
            for (auto line : city->connectedLines) {
                jsonLines.push_back(toId(line));
            }
            comps["CityComponent"] = {{"type", static_cast<int>(city->type)},
                                      {"connected_lines", jsonLines}};
        }

        if (auto *clickable = _registry.try_get<ClickableComponent>(entity)) {
            comps["ClickableComponent"] = {{"bounding_radius", clickable->boundingRadius.value}};
        }

        if (auto *renderable = _registry.try_get<RenderableComponent>(entity)) {
            comps["RenderableComponent"] = {
                {"radius", renderable->radius.value},
                {"color",
                 nlohmann::json::array(
                     {renderable->color.r, renderable->color.g, renderable->color.b,
                      renderable->color.a})},
                {"z_order", renderable->zOrder.value}};
        }

        if (auto *aabb = _registry.try_get<AABBComponent>(entity)) {
            const auto &bounds = aabb->bounds;
            comps["AABBComponent"] = {
                {"position",
                 nlohmann::json{{"x", bounds.position.x}, {"y", bounds.position.y}}},
                {"size", nlohmann::json{{"x", bounds.size.x}, {"y", bounds.size.y}}}};
        }

        if (_registry.all_of<SelectedComponent>(entity)) {
            comps["SelectedComponent"] = true;
        }

        if (_registry.all_of<VisualizePathComponent>(entity)) {
            comps["VisualizePathComponent"] = true;
        }

        if (_registry.all_of<TrainTag>(entity)) {
            comps["TrainTag"] = true;
        }

        if (auto *movement = _registry.try_get<TrainMovementComponent>(entity)) {
            comps["TrainMovementComponent"] = {{"state", static_cast<int>(movement->state)},
                                               {"direction", static_cast<int>(movement->direction)},
                                               {"assigned_line", toId(movement->assignedLine)},
                                               {"distance_along_curve",
                                                movement->distanceAlongCurve},
                                               {"stop_timer", movement->stopTimer}};
        }

        if (auto *physics = _registry.try_get<TrainPhysicsComponent>(entity)) {
            comps["TrainPhysicsComponent"] = {{"max_speed", physics->maxSpeed},
                                              {"current_speed", physics->currentSpeed},
                                              {"acceleration", physics->acceleration}};
        }

        if (auto *capacity = _registry.try_get<TrainCapacityComponent>(entity)) {
            comps["TrainCapacityComponent"] = {{"capacity", capacity->capacity},
                                               {"current_load", capacity->currentLoad}};
        }

        if (auto *atStation = _registry.try_get<AtStationComponent>(entity)) {
            comps["AtStationComponent"] = {{"station_entity", toId(atStation->stationEntity)}};
        }

        if (auto *approach = _registry.try_get<StationApproachComponent>(entity)) {
            comps["StationApproachComponent"] = {
                {"approach_start",
                 nlohmann::json{{"x", approach->approachCurveStart.x},
                                {"y", approach->approachCurveStart.y}}},
                {"approach_control",
                 nlohmann::json{{"x", approach->approachCurveControl.x},
                                {"y", approach->approachCurveControl.y}}},
                {"deceleration_progress", approach->decelerationProgress},
                {"deceleration_distance", approach->decelerationDistance}};
        }

        if (auto *passenger = _registry.try_get<PassengerComponent>(entity)) {
            comps["PassengerComponent"] = {
                {"origin_station", toId(passenger->originStation)},
                {"destination_station", toId(passenger->destinationStation)},
                {"state", static_cast<int>(passenger->state)},
                {"current_container", toId(passenger->currentContainer)}};
        }

        if (auto *path = _registry.try_get<PathComponent>(entity)) {
            nlohmann::json nodes = nlohmann::json::array();
            for (auto node : path->nodes) {
                nodes.push_back(toId(node));
            }
            comps["PathComponent"] = {{"nodes", nodes},
                                      {"current_node_index", path->currentNodeIndex}};
        }

        if (auto *animation = _registry.try_get<PassengerSpawnAnimationComponent>(entity)) {
            comps["PassengerSpawnAnimationComponent"] = {
                {"progress", animation->progress},
                {"duration", animation->duration},
                {"origin_city", toId(animation->originCity)},
                {"destination_city", toId(animation->destinationCity)}};
        }

        if (auto *line = _registry.try_get<LineComponent>(entity)) {
            nlohmann::json points = nlohmann::json::array();
            for (const auto &point : line->points) {
                nlohmann::json pointJson;
                pointJson["type"] = static_cast<int>(point.type == LinePointType::STOP ? 0 : 1);
                pointJson["position"] = {{"x", point.position.x}, {"y", point.position.y}};
                pointJson["station_entity"] = toId(point.stationEntity);
                pointJson["snap_side"] = point.snapSide;
                if (point.snapInfo) {
                    pointJson["snap_info"] = {{"entity", toId(point.snapInfo->snappedToEntity)},
                                              {"point_index", point.snapInfo->snappedToPointIndex}};
                } else {
                    pointJson["snap_info"] = nullptr;
                }
                points.push_back(pointJson);
            }

            nlohmann::json curvePoints = nlohmann::json::array();
            for (const auto &cp : line->curvePoints) {
                curvePoints.push_back({{"x", cp.x}, {"y", cp.y}});
            }

            nlohmann::json offsets = nlohmann::json::array();
            for (const auto &offset : line->pathOffsets) {
                offsets.push_back({{"x", offset.x}, {"y", offset.y}});
            }

            nlohmann::json stops = nlohmann::json::array();
            for (const auto &stop : line->stops) {
                stops.push_back({{"station_entity", toId(stop.stationEntity)},
                                 {"distance_along_curve", stop.distanceAlongCurve}});
            }

            nlohmann::json segmentIndices = nlohmann::json::array();
            for (size_t index : line->curveSegmentIndices) {
                segmentIndices.push_back(index);
            }

            comps["LineComponent"] = {
                {"color",
                 nlohmann::json::array({line->color.r, line->color.g, line->color.b,
                                        line->color.a})},
                {"points", points},
                {"curve_points", curvePoints},
                {"path_offsets", offsets},
                {"stops", stops},
                {"curve_segment_indices", segmentIndices},
                {"total_distance", line->totalDistance},
                {"thickness", line->thickness.value}};
        }

        if (auto *lineEditing = _registry.try_get<LineEditingComponent>(entity)) {
            nlohmann::json editing;
            editing["selected_point_index"] = lineEditing->selectedPointIndex
                                                  ? nlohmann::json(*lineEditing->selectedPointIndex)
                                                  : nlohmann::json(nullptr);
            editing["dragged_point_index"] = lineEditing->draggedPointIndex
                                                 ? nlohmann::json(*lineEditing->draggedPointIndex)
                                                 : nlohmann::json(nullptr);
            editing["original_point_position"] =
                lineEditing->originalPointPosition
                    ? nlohmann::json{{"x", lineEditing->originalPointPosition->x},
                                     {"y", lineEditing->originalPointPosition->y}}
                    : nlohmann::json(nullptr);
            editing["snap_position"] = lineEditing->snapPosition
                                           ? nlohmann::json{{"x", lineEditing->snapPosition->x},
                                                            {"y", lineEditing->snapPosition->y}}
                                           : nlohmann::json(nullptr);
            if (lineEditing->snapInfo) {
                editing["snap_info"] = {
                    {"entity", toId(lineEditing->snapInfo->snappedToEntity)},
                    {"point_index", lineEditing->snapInfo->snappedToPointIndex}};
            } else {
                editing["snap_info"] = nullptr;
            }
            editing["snap_side"] = lineEditing->snapSide;
            editing["snap_tangent"] = lineEditing->snapTangent
                                          ? nlohmann::json{{"x", lineEditing->snapTangent->x},
                                                           {"y", lineEditing->snapTangent->y}}
                                          : nlohmann::json(nullptr);

            comps["LineEditingComponent"] = editing;
        }

        if (!comps.empty()) {
            nlohmann::json record;
            record["id"] = toId(entity);
            record["components"] = std::move(comps);
            entities.push_back(std::move(record));
        }
    }
    return entities;
}

std::unordered_map<SaveLoadSystem::EntityId, entt::entity>
SaveLoadSystem::deserializeEntities(const nlohmann::json &data,
                                    std::vector<entt::entity> &lineEntities) {
    std::unordered_map<EntityId, entt::entity> idMap;
    std::vector<PendingEntity> pending;

    if (!data.is_array()) {
        return idMap;
    }

    pending.reserve(data.size());
    for (const auto &entry : data) {
        EntityId id = entry.value("id", 0);
        auto entity = _registry.create();
        idMap[id] = entity;
        pending.push_back({entity, entry.value("components", nlohmann::json::object())});
    }

    for (const auto &pendingEntity : pending) {
        const auto &components = pendingEntity.components;
        entt::entity entity = pendingEntity.entity;

        if (components.contains("PositionComponent")) {
            auto position = deserializeVec2(components["PositionComponent"]);
            _registry.emplace<PositionComponent>(entity, position);
        }

        if (components.contains("NameComponent")) {
            _registry.emplace<NameComponent>(entity,
                                             components["NameComponent"].value("name", ""));
        }

        if (components.contains("CityComponent")) {
            const auto &dataCity = components["CityComponent"];
            auto &city = _registry.emplace<CityComponent>(entity);
            city.type = cityTypeFromInt(dataCity.value("type", 1));
            city.connectedLines.clear();
            if (dataCity.contains("connected_lines")) {
                for (const auto &lineId : dataCity["connected_lines"]) {
                    city.connectedLines.push_back(toEntity(lineId.get<EntityId>(), idMap));
                }
            }
        }

        if (components.contains("ClickableComponent")) {
            auto radius = components["ClickableComponent"].value("bounding_radius", 0.0f);
            _registry.emplace<ClickableComponent>(entity, Radius{radius});
        }

        if (components.contains("RenderableComponent")) {
            const auto &renderableData = components["RenderableComponent"];
            auto &renderable = _registry.emplace<RenderableComponent>(entity, Radius{0.0f},
                                                                      sf::Color::White, ZOrder{0});
            renderable.radius = Radius{renderableData.value("radius", 0.0f)};
            if (renderableData.contains("color") && renderableData["color"].is_array()
                && renderableData["color"].size() == 4) {
                renderable.color = deserializeColor(renderableData["color"]);
            }
            renderable.zOrder = ZOrder{renderableData.value("z_order", 0)};
        }

        if (components.contains("AABBComponent")) {
            const auto &aabbData = components["AABBComponent"];
            sf::Vector2f position = {aabbData.value("left", 0.0f),
                                     aabbData.value("top", 0.0f)};
            sf::Vector2f size = {aabbData.value("width", 0.0f),
                                 aabbData.value("height", 0.0f)};
            if (aabbData.contains("position")) {
                position = deserializeVec2(aabbData["position"]);
            }
            if (aabbData.contains("size")) {
                size = deserializeVec2(aabbData["size"]);
            }
            sf::FloatRect rect;
            rect.position = position;
            rect.size = size;
            _registry.emplace<AABBComponent>(entity, rect);
        }

        if (components.contains("SelectedComponent")) {
            _registry.emplace<SelectedComponent>(entity);
        }

        if (components.contains("VisualizePathComponent")) {
            _registry.emplace<VisualizePathComponent>(entity);
        }

        if (components.contains("TrainTag")) {
            _registry.emplace<TrainTag>(entity);
        }

        if (components.contains("TrainMovementComponent")) {
            const auto &dataMovement = components["TrainMovementComponent"];
            auto &movement = _registry.emplace<TrainMovementComponent>(entity);
            movement.state = trainStateFromInt(dataMovement.value("state", 0));
            movement.direction = trainDirectionFromInt(dataMovement.value("direction", 0));
            movement.assignedLine = toEntity(dataMovement.value("assigned_line", 0), idMap);
            movement.distanceAlongCurve = dataMovement.value("distance_along_curve", 0.0f);
            movement.stopTimer = dataMovement.value("stop_timer", Constants::TRAIN_STOP_DURATION);
        }

        if (components.contains("TrainPhysicsComponent")) {
            const auto &dataPhysics = components["TrainPhysicsComponent"];
            auto &physics = _registry.emplace<TrainPhysicsComponent>(entity);
            physics.maxSpeed = dataPhysics.value("max_speed", Constants::TRAIN_MAX_SPEED);
            physics.currentSpeed = dataPhysics.value("current_speed", 0.0f);
            physics.acceleration = dataPhysics.value("acceleration", Constants::TRAIN_ACCELERATION);
        }

        if (components.contains("TrainCapacityComponent")) {
            const auto &dataCapacity = components["TrainCapacityComponent"];
            auto &capacity = _registry.emplace<TrainCapacityComponent>(entity);
            capacity.capacity = dataCapacity.value("capacity", Constants::TRAIN_CAPACITY);
            capacity.currentLoad = dataCapacity.value("current_load", 0);
        }

        if (components.contains("AtStationComponent")) {
            const auto &dataAtStation = components["AtStationComponent"];
            _registry.emplace<AtStationComponent>(
                entity, toEntity(dataAtStation.value("station_entity", 0), idMap));
        }

        if (components.contains("StationApproachComponent")) {
            const auto &dataApproach = components["StationApproachComponent"];
            auto &approach = _registry.emplace<StationApproachComponent>(entity);
            approach.approachCurveStart = deserializeVec2(dataApproach["approach_start"]);
            approach.approachCurveControl = deserializeVec2(dataApproach["approach_control"]);
            approach.decelerationProgress = dataApproach.value("deceleration_progress", 0.0f);
            approach.decelerationDistance = dataApproach.value("deceleration_distance", 0.0f);
        }

        if (components.contains("PassengerComponent")) {
            const auto &dataPassenger = components["PassengerComponent"];
            auto &passenger = _registry.emplace<PassengerComponent>(entity);
            passenger.originStation = toEntity(dataPassenger.value("origin_station", 0), idMap);
            passenger.destinationStation =
                toEntity(dataPassenger.value("destination_station", 0), idMap);
            passenger.state =
                passengerStateFromInt(dataPassenger.value("state", 0));
            passenger.currentContainer =
                toEntity(dataPassenger.value("current_container", 0), idMap);
        }

        if (components.contains("PathComponent")) {
            const auto &dataPath = components["PathComponent"];
            auto &path = _registry.emplace<PathComponent>(entity);
            path.nodes.clear();
            if (dataPath.contains("nodes")) {
                for (const auto &nodeId : dataPath["nodes"]) {
                    path.nodes.push_back(toEntity(nodeId.get<EntityId>(), idMap));
                }
            }
            path.currentNodeIndex = dataPath.value("current_node_index", 0);
        }

        if (components.contains("PassengerSpawnAnimationComponent")) {
            const auto &dataAnimation = components["PassengerSpawnAnimationComponent"];
            auto &animation = _registry.emplace<PassengerSpawnAnimationComponent>(entity);
            animation.progress = dataAnimation.value("progress", 0.0f);
            animation.duration = dataAnimation.value("duration", 1.0f);
            animation.originCity = toEntity(dataAnimation.value("origin_city", 0), idMap);
            animation.destinationCity =
                toEntity(dataAnimation.value("destination_city", 0), idMap);
        }

        if (components.contains("LineComponent")) {
            const auto &dataLine = components["LineComponent"];
            auto &line = _registry.emplace<LineComponent>(entity);

            if (dataLine.contains("color") && dataLine["color"].is_array()
                && dataLine["color"].size() == 4) {
                line.color = deserializeColor(dataLine["color"]);
            }

            line.points.clear();
            if (dataLine.contains("points")) {
                for (const auto &pointJson : dataLine["points"]) {
                    LinePoint point;
                    point.type = linePointTypeFromInt(pointJson.value("type", 0));
                    point.position = deserializeVec2(pointJson["position"]);
                    point.stationEntity = toEntity(pointJson.value("station_entity", 0), idMap);
                    point.snapSide = pointJson.value("snap_side", 0.0f);
                    if (pointJson.contains("snap_info") && !pointJson["snap_info"].is_null()) {
                        SnapInfo snap;
                        snap.snappedToEntity =
                            toEntity(pointJson["snap_info"].value("entity", 0), idMap);
                        snap.snappedToPointIndex =
                            pointJson["snap_info"].value("point_index", static_cast<size_t>(0));
                        point.snapInfo = snap;
                    } else {
                        point.snapInfo = std::nullopt;
                    }
                    line.points.push_back(point);
                }
            }

            line.curvePoints.clear();
            if (dataLine.contains("curve_points")) {
                for (const auto &cp : dataLine["curve_points"]) {
                    line.curvePoints.push_back(deserializeVec2(cp));
                }
            }

            line.pathOffsets.clear();
            if (dataLine.contains("path_offsets")) {
                for (const auto &offsetJson : dataLine["path_offsets"]) {
                    line.pathOffsets.push_back(deserializeVec2(offsetJson));
                }
            }

            line.stops.clear();
            if (dataLine.contains("stops")) {
                for (const auto &stopJson : dataLine["stops"]) {
                    StopInfo stop;
                    stop.stationEntity = toEntity(stopJson.value("station_entity", 0), idMap);
                    stop.distanceAlongCurve = stopJson.value("distance_along_curve", 0.0f);
                    line.stops.push_back(stop);
                }
            }

            line.curveSegmentIndices.clear();
            if (dataLine.contains("curve_segment_indices")) {
                for (const auto &index : dataLine["curve_segment_indices"]) {
                    line.curveSegmentIndices.push_back(index.get<size_t>());
                }
            }

            line.totalDistance = dataLine.value("total_distance", 0.0f);
            line.thickness = Thickness{dataLine.value("thickness", Constants::DEFAULT_LINE_THICKNESS)};
            line.sharedSegments.clear();
            lineEntities.push_back(entity);
        }

        if (components.contains("LineEditingComponent")) {
            const auto &dataEditing = components["LineEditingComponent"];
            auto &editing = _registry.emplace<LineEditingComponent>(entity);
            editing.selectedPointIndex =
                deserializeOptionalIndex(dataEditing, "selected_point_index");
            editing.draggedPointIndex =
                deserializeOptionalIndex(dataEditing, "dragged_point_index");
            editing.originalPointPosition =
                dataEditing.contains("original_point_position")
                    && !dataEditing["original_point_position"].is_null()
                    ? std::optional<sf::Vector2f>(
                          deserializeVec2(dataEditing["original_point_position"]))
                    : std::nullopt;

            editing.snapPosition =
                dataEditing.contains("snap_position") && !dataEditing["snap_position"].is_null()
                    ? std::optional<sf::Vector2f>(
                          deserializeVec2(dataEditing["snap_position"]))
                    : std::nullopt;

            if (dataEditing.contains("snap_info") && !dataEditing["snap_info"].is_null()) {
                SnapInfo snap;
                snap.snappedToEntity =
                    toEntity(dataEditing["snap_info"].value("entity", 0), idMap);
                snap.snappedToPointIndex =
                    dataEditing["snap_info"].value("point_index", static_cast<size_t>(0));
                editing.snapInfo = snap;
            } else {
                editing.snapInfo.reset();
            }
            editing.snapSide = dataEditing.value("snap_side", 0.0f);
            editing.snapTangent =
                dataEditing.contains("snap_tangent") && !dataEditing["snap_tangent"].is_null()
                    ? std::optional<sf::Vector2f>(
                          deserializeVec2(dataEditing["snap_tangent"]))
                    : std::nullopt;
        }
    }

    return idMap;
}

nlohmann::json SaveLoadSystem::serializeGameState() const {
    nlohmann::json data;
    data["interaction_mode"] = static_cast<int>(_gameState.currentInteractionMode);
    data["app_state"] = static_cast<int>(_gameState.currentAppState);
    if (_gameState.selectedEntity && _registry.valid(*_gameState.selectedEntity)) {
        data["selected_entity"] = toId(*_gameState.selectedEntity);
    } else {
        data["selected_entity"] = nullptr;
    }
    if (_gameState.passengerOriginStation
        && _registry.valid(*_gameState.passengerOriginStation)) {
        data["passenger_origin_station"] = toId(*_gameState.passengerOriginStation);
    } else {
        data["passenger_origin_station"] = nullptr;
    }
    data["time_multiplier"] = _gameState.timeMultiplier;
    data["pre_edit_time_multiplier"] = _gameState.preEditTimeMultiplier;
    data["total_elapsed_time"] = _gameState.totalElapsedTime.asSeconds();
    data["elevation_checks_enabled"] = _gameState.elevationChecksEnabled;
    return data;
}

void SaveLoadSystem::applyGameState(const nlohmann::json &data,
                                    const std::unordered_map<EntityId, entt::entity> &map) {
    _gameState.currentInteractionMode =
        interactionModeFromInt(data.value("interaction_mode", 0));
    _gameState.currentAppState = appStateFromInt(data.value("app_state", 1));

    if (data.contains("selected_entity") && !data["selected_entity"].is_null()) {
        auto entity = toEntity(data["selected_entity"].get<EntityId>(), map);
        if (entity != entt::null && _registry.valid(entity)) {
            _gameState.selectedEntity = entity;
        } else {
            _gameState.selectedEntity.reset();
        }
    } else {
        _gameState.selectedEntity.reset();
    }

    if (data.contains("passenger_origin_station")
        && !data["passenger_origin_station"].is_null()) {
        auto entity = toEntity(data["passenger_origin_station"].get<EntityId>(), map);
        if (entity != entt::null && _registry.valid(entity)) {
            _gameState.passengerOriginStation = entity;
        } else {
            _gameState.passengerOriginStation.reset();
        }
    } else {
        _gameState.passengerOriginStation.reset();
    }

    _gameState.timeMultiplier = data.value("time_multiplier", 1.0f);
    _gameState.preEditTimeMultiplier = data.value("pre_edit_time_multiplier", 1.0f);
    _gameState.totalElapsedTime = sf::seconds(data.value("total_elapsed_time", 0.0f));
    _gameState.elevationChecksEnabled = data.value("elevation_checks_enabled", true);
}

nlohmann::json SaveLoadSystem::serializeCityPlacement() const {
    nlohmann::json data;
    const auto state = _cityPlacementSystem.getSerializedState();

    data["weights"] = {{"water_access", state.weights.waterAccess},
                       {"land_expandability", state.weights.landExpandability},
                       {"city_proximity", state.weights.cityProximity},
                       {"randomness", state.weights.randomness}};

    nlohmann::json placedCities = nlohmann::json::array();
    for (const auto &city : state.placedCities) {
        placedCities.push_back({{"x", city.position.x},
                                {"y", city.position.y},
                                {"type", static_cast<int>(city.type)}});
    }
    data["placed_cities"] = placedCities;

    nlohmann::json terrainCache = nlohmann::json::array();
    for (auto terrain : state.terrainCache) {
        terrainCache.push_back(static_cast<int>(terrain));
    }
    data["terrain_cache"] = terrainCache;

    nlohmann::json distCapital = nlohmann::json::array();
    for (int distance : state.distanceToNearestCapital) {
        distCapital.push_back(distance);
    }
    data["distance_to_nearest_capital"] = distCapital;

    nlohmann::json distTown = nlohmann::json::array();
    for (int distance : state.distanceToNearestTown) {
        distTown.push_back(distance);
    }
    data["distance_to_nearest_town"] = distTown;

    auto serializeMap = [](const std::vector<float> &values) {
        nlohmann::json arr = nlohmann::json::array();
        for (float value : values) arr.push_back(value);
        return arr;
    };

    nlohmann::json suitability;
    suitability["water"] = serializeMap(state.suitabilityMaps.water);
    suitability["expandability"] = serializeMap(state.suitabilityMaps.expandability);
    suitability["city_proximity"] = serializeMap(state.suitabilityMaps.cityProximity);
    suitability["noise"] = serializeMap(state.suitabilityMaps.noise);
    suitability["final"] = serializeMap(state.suitabilityMaps.final);
    suitability["town_proximity"] = serializeMap(state.suitabilityMaps.townProximity);
    suitability["suburb_proximity"] = serializeMap(state.suitabilityMaps.suburbProximity);
    suitability["town_final"] = serializeMap(state.suitabilityMaps.townFinal);
    suitability["suburb_final"] = serializeMap(state.suitabilityMaps.suburbFinal);
    data["suitability_maps"] = suitability;

    data["time_since_last_city"] = state.timeSinceLastCity;
    data["current_spawn_interval"] = state.currentSpawnInterval;
    data["min_spawn_interval"] = state.minSpawnInterval;
    data["max_spawn_interval"] = state.maxSpawnInterval;
    data["max_cities"] = state.maxCities;
    data["initial_placement_done"] = state.initialPlacementDone;
    data["last_placement_success"] = state.lastPlacementSuccess;
    data["next_city_type"] = static_cast<int>(state.nextCityType);

    data["debug_info"] = {{"time_to_next_placement", state.debugInfo.timeToNextPlacement},
                          {"next_city_type", static_cast<int>(state.debugInfo.nextCityType)},
                          {"last_placement_success", state.debugInfo.lastPlacementSuccess},
                          {"town_suitability_percentage",
                           state.debugInfo.townSuitabilityPercentage},
                          {"suburb_suitability_percentage",
                           state.debugInfo.suburbSuitabilityPercentage}};
    data["rng_state"] = state.rngState;
    return data;
}

void SaveLoadSystem::applyCityPlacement(const nlohmann::json &data) {
    CityPlacementSerializedState state;
    const auto &weights = data.value("weights", nlohmann::json::object());
    state.weights.waterAccess = weights.value("water_access", state.weights.waterAccess);
    state.weights.landExpandability =
        weights.value("land_expandability", state.weights.landExpandability);
    state.weights.cityProximity = weights.value("city_proximity", state.weights.cityProximity);
    state.weights.randomness = weights.value("randomness", state.weights.randomness);

    state.placedCities.clear();
    if (data.contains("placed_cities")) {
        for (const auto &cityJson : data["placed_cities"]) {
            PlacedCityInfo info;
            info.position.x = cityJson.value("x", 0);
            info.position.y = cityJson.value("y", 0);
            info.type = cityTypeFromInt(cityJson.value("type", 1));
            state.placedCities.push_back(info);
        }
    }

    state.terrainCache.clear();
    if (data.contains("terrain_cache")) {
        for (const auto &terrainValue : data["terrain_cache"]) {
            state.terrainCache.push_back(terrainFromInt(terrainValue.get<int>()));
        }
    }

    state.distanceToNearestCapital.clear();
    if (data.contains("distance_to_nearest_capital")) {
        for (const auto &distance : data["distance_to_nearest_capital"]) {
            state.distanceToNearestCapital.push_back(distance.get<int>());
        }
    }

    state.distanceToNearestTown.clear();
    if (data.contains("distance_to_nearest_town")) {
        for (const auto &distance : data["distance_to_nearest_town"]) {
            state.distanceToNearestTown.push_back(distance.get<int>());
        }
    }

    auto loadMap = [](const nlohmann::json &json, std::vector<float> &target) {
        target.clear();
        if (json.is_array()) {
            for (const auto &value : json) {
                target.push_back(value.get<float>());
            }
        }
    };

    if (data.contains("suitability_maps")) {
        const auto &maps = data["suitability_maps"];
        loadMap(maps.value("water", nlohmann::json::array()), state.suitabilityMaps.water);
        loadMap(maps.value("expandability", nlohmann::json::array()),
                state.suitabilityMaps.expandability);
        loadMap(maps.value("city_proximity", nlohmann::json::array()),
                state.suitabilityMaps.cityProximity);
        loadMap(maps.value("noise", nlohmann::json::array()), state.suitabilityMaps.noise);
        loadMap(maps.value("final", nlohmann::json::array()), state.suitabilityMaps.final);
        loadMap(maps.value("town_proximity", nlohmann::json::array()),
                state.suitabilityMaps.townProximity);
        loadMap(maps.value("suburb_proximity", nlohmann::json::array()),
                state.suitabilityMaps.suburbProximity);
        loadMap(maps.value("town_final", nlohmann::json::array()),
                state.suitabilityMaps.townFinal);
        loadMap(maps.value("suburb_final", nlohmann::json::array()),
                state.suitabilityMaps.suburbFinal);
    }

    state.timeSinceLastCity = data.value("time_since_last_city", 0.0f);
    state.currentSpawnInterval = data.value("current_spawn_interval", 0.0f);
    state.minSpawnInterval = data.value("min_spawn_interval", state.minSpawnInterval);
    state.maxSpawnInterval = data.value("max_spawn_interval", state.maxSpawnInterval);
    state.maxCities = data.value("max_cities", state.maxCities);
    state.initialPlacementDone = data.value("initial_placement_done", false);
    state.lastPlacementSuccess = data.value("last_placement_success", false);
    state.nextCityType = cityTypeFromInt(data.value("next_city_type", 1));

    const auto &debug = data.value("debug_info", nlohmann::json::object());
    state.debugInfo.timeToNextPlacement =
        debug.value("time_to_next_placement", state.debugInfo.timeToNextPlacement);
    state.debugInfo.nextCityType =
        cityTypeFromInt(debug.value("next_city_type", 1));
    state.debugInfo.lastPlacementSuccess =
        debug.value("last_placement_success", state.debugInfo.lastPlacementSuccess);
    state.debugInfo.townSuitabilityPercentage =
        debug.value("town_suitability_percentage", state.debugInfo.townSuitabilityPercentage);
    state.debugInfo.suburbSuitabilityPercentage =
        debug.value("suburb_suitability_percentage", state.debugInfo.suburbSuitabilityPercentage);

    state.rngState = data.value("rng_state", std::string());
    _cityPlacementSystem.applySerializedState(state);
}

nlohmann::json SaveLoadSystem::serializeScore() const {
    nlohmann::json data;
    auto view = _registry.view<const GameScoreComponent>();
    for (auto entity : view) {
        data["score"] = view.get<const GameScoreComponent>(entity).score;
        break;
    }
    return data;
}

void SaveLoadSystem::applyScore(const nlohmann::json &data) {
    auto view = _registry.view<GameScoreComponent>();
    for (auto entity : view) {
        auto &score = view.get<GameScoreComponent>(entity);
        score.score = data.value("score", score.score);
        break;
    }
}

nlohmann::json SaveLoadSystem::serializePassengerSpawn() const {
    nlohmann::json data;
    data["spawn_timer"] = _passengerSpawnSystem.getSpawnTimer().asSeconds();
    data["spawn_interval"] = _passengerSpawnSystem.getSpawnInterval().asSeconds();
    return data;
}

void SaveLoadSystem::applyPassengerSpawn(const nlohmann::json &data) {
    _passengerSpawnSystem.setSpawnTimer(sf::seconds(data.value("spawn_timer", 5.0f)));
    _passengerSpawnSystem.setSpawnInterval(sf::seconds(data.value("spawn_interval", 5.0f)));
}

nlohmann::json SaveLoadSystem::serializeCamera() const {
    nlohmann::json data;
    const auto &view = _camera.getView();
    data["center"] = {{"x", view.getCenter().x}, {"y", view.getCenter().y}};
    data["size"] = {{"x", view.getSize().x}, {"y", view.getSize().y}};
    return data;
}

void SaveLoadSystem::applyCamera(const nlohmann::json &data) {
    auto &view = _camera.getViewToModify();
    if (data.contains("center")) {
        auto center = deserializeVec2(data["center"]);
        view.setCenter(center);
    }
    if (data.contains("size")) {
        auto size = deserializeVec2(data["size"]);
        view.setSize(size);
    }
}

void SaveLoadSystem::rebuildSharedSegments(const std::vector<entt::entity> &lineEntities) {
    for (auto entity : lineEntities) {
        if (_registry.valid(entity)) {
            _eventBus.enqueue<LineModifiedEvent>({entity});
        }
    }
}

void SaveLoadSystem::clearExistingEntities() {
    if (_registry.ctx().contains<ActiveLine>()) {
        _registry.ctx().erase<ActiveLine>();
    }
    if (_registry.ctx().contains<LinePreview>()) {
        _registry.ctx().erase<LinePreview>();
    }
    if (_registry.ctx().contains<SharedSegmentsContext>()) {
        _registry.ctx().erase<SharedSegmentsContext>();
    }

    std::vector<entt::entity> toDestroy;
    auto &entityStorage = _registry.storage<entt::entity>();
    for (auto [entity] : entityStorage.each()) {
        if (_registry.any_of<ChunkPositionComponent>(entity)) continue;
        if (_registry.any_of<GameScoreComponent>(entity)) continue;
        if (_registry.any_of<WorldStateComponent>(entity)) continue;
        toDestroy.push_back(entity);
    }
    _registry.destroy(toDestroy.begin(), toDestroy.end());
}
