#pragma once

#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>

#include "../core/Components.h"

class TerrainRenderSystem {
public:
    TerrainRenderSystem();

    void render(entt::registry& registry, sf::RenderTarget& target, const sf::View& view);
    void setVisualizeNoise(bool visualize) { _visualizeNoise = visualize; }
private:
    sf::RectangleShape _cellShape;
    bool _visualizeNoise;
    bool _visualizeNoiseStateChanged = false;

    const WorldGridComponent& getWorldGridSettings(entt::registry& registry);
    void buildChunkMesh(ChunkComponent& chunk, const WorldGridComponent& worldGrid);
};