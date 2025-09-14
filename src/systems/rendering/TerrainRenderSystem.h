#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <vector>
#include <map>
#include <memory>

struct ChunkPositionComponent;
struct ChunkTerrainComponent;
struct ChunkMeshComponent;
struct WorldGenParams;
struct SuitabilityMaps;
enum class TerrainType;

class TerrainRenderSystem {
public:
    enum class SuitabilityMapType {
        None,
        Water,
        Expandability,
        CityProximity,
        Final
    };

    TerrainRenderSystem();

    void updateMeshes(entt::registry &registry, const WorldGenParams &worldParams);
    void render(const entt::registry &registry, sf::RenderTarget &target, const sf::View &view, const WorldGenParams &worldParams);

    void setVisualizeChunkBorders(bool visualize) noexcept { _visualizeChunkBorders = visualize; }
    void setVisualizeCellBorders(bool visualize) noexcept { _visualizeCellBorders = visualize; }
    void setVisualizeSuitabilityMap(bool visualize) noexcept { _visualizeSuitabilityMap = visualize; }
    void setSuitabilityMapData(const SuitabilityMaps *maps, const std::vector<TerrainType> *terrainCache, const WorldGenParams &worldParams);
    void setSuitabilityMapType(SuitabilityMapType type) { _suitabilityMapType = type; }
    void setLodEnabled(bool enabled) noexcept;
    void regenerateSuitabilityMaps(const WorldGenParams &worldParams);

private:
    sf::RectangleShape _cellShape;
    bool _visualizeChunkBorders = false;
    bool _visualizeCellBorders = false;
    bool _visualizeSuitabilityMap = false;
    bool _isLodEnabled = true;
    const SuitabilityMaps *_suitabilityMaps = nullptr;
    const std::vector<TerrainType> *_terrainCache = nullptr;
    SuitabilityMapType _suitabilityMapType = SuitabilityMapType::None;
    std::vector<bool> m_visited;
    std::map<SuitabilityMapType, std::unique_ptr<sf::RenderTexture>> _suitabilityMapTextures;
    bool _suitabilityMapsDirty = true;

    void buildAllChunkMeshes(const ChunkPositionComponent &chunkPos,
                             const ChunkTerrainComponent &chunkTerrain,
                             ChunkMeshComponent &chunkMesh, const WorldGenParams &worldParams);
};