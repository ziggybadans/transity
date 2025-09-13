#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <vector>

struct ChunkPositionComponent;
struct ChunkTerrainComponent;
struct ChunkMeshComponent;
struct WorldGenParams;
struct SuitabilityMaps;

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
    void setSuitabilityMapData(const SuitabilityMaps *maps) { _suitabilityMaps = maps; }
    void setSuitabilityMapType(SuitabilityMapType type) { _suitabilityMapType = type; }
    void setLodEnabled(bool enabled) noexcept;

private:
    sf::RectangleShape _cellShape;
    bool _visualizeChunkBorders = false;
    bool _visualizeCellBorders = false;
    bool _visualizeSuitabilityMap = false;
    bool _isLodEnabled = true;
    const SuitabilityMaps *_suitabilityMaps = nullptr;
    SuitabilityMapType _suitabilityMapType = SuitabilityMapType::None;
    std::vector<bool> m_visited;

    void buildAllChunkMeshes(const ChunkPositionComponent &chunkPos,
                             const ChunkTerrainComponent &chunkTerrain,
                             ChunkMeshComponent &chunkMesh, const WorldGenParams &worldParams);
};