#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include "../core/Components.h"

class TerrainRenderSystem {
public:
    TerrainRenderSystem();

    // This is now a simulation-side system that modifies the registry
    void updateMeshes(entt::registry &registry);

    // Render is now const-correct and does not modify the registry
    void render(const entt::registry &registry, sf::RenderTarget &target, const sf::View &view);

    void setVisualizeChunkBorders(bool visualize) { _visualizeChunkBorders = visualize; }
    void setVisualizeCellBorders(bool visualize) { _visualizeCellBorders = visualize; }
    void setLodEnabled(bool enabled);

private:
    sf::RectangleShape _cellShape;
    bool _visualizeChunkBorders = false;
    bool _visualizeCellBorders = false;
    bool _isLodEnabled = true;

    const WorldGridComponent &getWorldGridSettings(const entt::registry &registry);
    void buildAllChunkMeshes(const ChunkComponent &chunk, ChunkMeshComponent &chunkMesh,
                             const WorldGridComponent &worldGrid);
};