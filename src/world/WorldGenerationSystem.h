#pragma once

#include <entt/entt.hpp>

#include "FastNoiseLite.h"
#include "../core/Components.h"
#include "WorldData.h"

class WorldGenerationSystem {
public:
    WorldGenerationSystem(entt::registry& registry);

    void configureNoise ();

    void generateChunk(entt::entity chunkEntity);
    void generateWorld(int numChunksX, int numChunksY);
    sf::Vector2f getWorldSize();

    entt::registry& getRegistry() { return _registry; }

    const WorldGenParams& getParams() const { return _params; }
    void setParams(const WorldGenParams& params);

private:
    entt::registry& _registry;

    FastNoiseLite _noiseGenerator;
    WorldGenParams _params;

    std::vector<sf::Vector2f> _islandShape;
    
    const WorldGridComponent& getWorldGridSettings();
    std::vector<sf::Vector2f> generateIslandBaseShape();
    std::vector<sf::Vector2f> distortCoastline(const std::vector<sf::Vector2f>& baseShape);
};