#pragma once

#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>

#include "../core/Components.h"

class TerrainRenderSystem {
public:
    TerrainRenderSystem();

    void render(entt::registry& registry, sf::RenderTarget& target, const sf::View& view);
    void setVisualizeChunkBorders(bool visualize) { _visualizeChunkBorders = visualize; } // ADD
    void setVisualizeCellBorders(bool visualize) { _visualizeCellBorders = visualize; }   // ADD
    void setVisualizeNoise(bool visualize) { 
        _visualizeNoise = visualize; 
        _visualizeNoiseStateChanged = true;
    }
// ...
private:
    sf::RectangleShape _cellShape;
    bool _visualizeNoise;
    bool _visualizeNoiseStateChanged = false;
    bool _visualizeChunkBorders = false; // ADD
    bool _visualizeCellBorders = false;  // ADD

    const WorldGridComponent& getWorldGridSettings(entt::registry& registry);
    void buildChunkMesh(ChunkComponent& chunk, const WorldGridComponent& worldGrid);
};