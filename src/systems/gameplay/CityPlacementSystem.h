#pragma once

#include "ecs/ISystem.h"
#include "entt/entt.hpp"
#include <SFML/System/Vector2.hpp>

class EntityFactory; // Forward-declare
class WorldGenerationSystem;

class CityPlacementSystem : public ISystem {
public:
    // Update the constructor signature
    explicit CityPlacementSystem(entt::registry &registry, EntityFactory &entityFactory,
                                 const WorldGenerationSystem &worldGenSystem);
    ~CityPlacementSystem();

    void placeCities(int numberOfCities);

private:
    sf::Vector2i findBestLocation(int mapWidth, int mapHeight,
                                  const std::vector<float> &suitabilityMap);
    void reduceSuitabilityAroundCity(int cityX, int cityY, int mapWidth, int mapHeight,
                                     std::vector<float> &suitabilityMap);

private:
    entt::registry &_registry;
    EntityFactory &_entityFactory;
    const WorldGenerationSystem &_worldGenSystem;
};