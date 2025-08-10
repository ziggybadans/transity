#pragma once

#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>

#include "../core/Components.h"

class TerrainRenderSystem {
public:
    TerrainRenderSystem();

    void render(entt::registry& registry, sf::RenderTarget& target, const sf::View& view);
    void setVisualizeChunkBorders(bool visualize) { _visualizeChunkBorders = visualize; }
    void setVisualizeCellBorders(bool visualize) { _visualizeCellBorders = visualize; }
    void setLodEnabled(entt::registry& registry, bool enabled);

private:
    sf::RectangleShape _cellShape;
    bool _visualizeChunkBorders = false;
    bool _visualizeCellBorders = false;
    bool _isLodEnabled = true;

    const WorldGridComponent& getWorldGridSettings(entt::registry& registry);
    void buildAllChunkMeshes(ChunkComponent& chunk, const WorldGridComponent& worldGrid);
};
