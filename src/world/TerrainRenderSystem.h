#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include "../core/Components.h"

class TerrainRenderSystem {
public:
    TerrainRenderSystem();

    
    void updateMeshes(entt::registry &registry);

    
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