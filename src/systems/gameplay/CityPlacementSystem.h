#pragma once

#include "ecs/ISystem.h"
#include "entt/entt.hpp"
#include <SFML/System/Vector2.hpp>

class ServiceLocator;

class CityPlacementSystem : public ISystem, public IUpdatable {
public:
    explicit CityPlacementSystem(ServiceLocator &serviceLocator);
    ~CityPlacementSystem() override;

    void update(sf::Time dt) override;

private:
    void placeCities(int numberOfCities);
    sf::Vector2i findBestLocation(int mapWidth, int mapHeight,
                                  const std::vector<float> &suitabilityMap);
    void reduceSuitabilityAroundCity(int cityX, int cityY, int mapWidth, int mapHeight,
                                     std::vector<float> &suitabilityMap);

private:
    ServiceLocator &_serviceLocator;
    bool _hasRun = false;
};