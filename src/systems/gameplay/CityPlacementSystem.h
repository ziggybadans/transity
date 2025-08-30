#pragma once

#include "ecs/ISystem.h"
#include "entt/entt.hpp"
#include <SFML/System/Vector2.hpp>
#include <vector>

class ServiceLocator;

struct PlacementWeights {
    float waterAccess = 0.4f;
    float landExpandability = 0.3f;
    float cityProximity = 0.3f;
};

struct SuitabilityMaps {
    std::vector<float> water;
    std::vector<float> expandability;
    std::vector<float> cityProximity;
    std::vector<float> final;
};

class CityPlacementSystem : public ISystem, public IUpdatable {
public:
    explicit CityPlacementSystem(ServiceLocator &serviceLocator);
    ~CityPlacementSystem() override;

    void update(sf::Time dt) override;

private:
    void placeCities(int numberOfCities);

    void calculateSuitabilityMaps(int mapWidth, int mapHeight, SuitabilityMaps &maps);
    void calculateWaterSuitability(int mapWidth, int mapHeight, std::vector<float> &map);
    void calculateExpandabilitySuitability(int mapWidth, int mapHeight, std::vector<float> &map);
    void updateCityProximitySuitability(int mapWidth, int mapHeight, std::vector<float> &map);
    void combineSuitabilityMaps(int mapWidth, int mapHeight, const SuitabilityMaps &maps,
                                const PlacementWeights &weights, std::vector<float> &finalMap);
    void normalizeMap(std::vector<float> &map);

    sf::Vector2i findBestLocation(int mapWidth, int mapHeight,
                                  const std::vector<float> &suitabilityMap);
    void reduceSuitabilityAroundCity(int cityX, int cityY, int mapWidth, int mapHeight,
                                     std::vector<float> &suitabilityMap);

private:
    ServiceLocator &_serviceLocator;
    bool _hasRun = false;
    PlacementWeights _weights;
    std::vector<sf::Vector2i> _placedCities;
};