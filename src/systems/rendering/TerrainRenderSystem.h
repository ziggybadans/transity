#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <vector>

struct ChunkPositionComponent;
struct ChunkTerrainComponent;
struct ChunkMeshComponent;
struct WorldGenParams;

class TerrainRenderSystem {
public:
    TerrainRenderSystem();

    void updateMeshes(entt::registry &registry, const WorldGenParams &worldParams);
    void render(const entt::registry &registry, sf::RenderTarget &target, const sf::View &view, const WorldGenParams &worldParams);

    void setVisualizeChunkBorders(bool visualize) noexcept { _visualizeChunkBorders = visualize; }
    void setVisualizeCellBorders(bool visualize) noexcept { _visualizeCellBorders = visualize; }
    void setLodEnabled(bool enabled) noexcept;

private:
    sf::RectangleShape _cellShape;
    bool _visualizeChunkBorders = false;
    bool _visualizeCellBorders = false;
    bool _isLodEnabled = true;
    std::vector<bool> m_visited;

    void buildAllChunkMeshes(const ChunkPositionComponent &chunkPos,
                             const ChunkTerrainComponent &chunkTerrain,
                             ChunkMeshComponent &chunkMesh, const WorldGenParams &worldParams);
};
