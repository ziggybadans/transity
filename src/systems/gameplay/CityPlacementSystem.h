#pragma once

#include "ecs/ISystem.h"
#include "entt/entt.hpp"
#include "components/WorldComponents.h"
#include "FastNoiseLite.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Time.hpp>
#include <vector>
#include <random>

struct LoadingState;
class WorldGenerationSystem;
class EntityFactory;
class Renderer;
class PerformanceMonitor;

struct PlacementWeights {
    float waterAccess = 0.20f;
    float landExpandability = 0.25f;
    float cityProximity = 0.35f;
    float randomness = 0.20f;
};

struct SuitabilityMaps {
    std::vector<float> water;
    std::vector<float> expandability;
    std::vector<float> cityProximity;
    std::vector<float> noise;
    std::vector<float> final;
};

class CityPlacementSystem : public ISystem, public IUpdatable {
public:
    explicit CityPlacementSystem(LoadingState& loadingState, WorldGenerationSystem& worldGenerationSystem, EntityFactory& entityFactory, Renderer& renderer, PerformanceMonitor& performanceMonitor);
    ~CityPlacementSystem() override;

    void init();
    void update(sf::Time dt) override; // This will now correctly override IUpdatable::update

    const SuitabilityMaps &getSuitabilityMaps() const;

private:
    void initialPlacement();
    bool placeNewCity();
    
    void precomputeTerrainCache(int mapWidth, int mapHeight);

    void calculateWaterSuitability(int mapWidth, int mapHeight, std::vector<float> &map);
    void calculateExpandabilitySuitability(int mapWidth, int mapHeight, std::vector<float> &map);
    void calculateNoiseSuitability(int mapWidth, int mapHeight, std::vector<float> &map);

    void updateDistanceMap(const sf::Vector2i &newCity, int mapWidth, int mapHeight);
    void calculateProximitySuitability(int mapWidth, int mapHeight, std::vector<float> &map);

    void combineSuitabilityMaps(int mapWidth, int mapHeight, const SuitabilityMaps &maps,
                                const PlacementWeights &weights, std::vector<float> &finalMap);
    void normalizeMap(std::vector<float> &map);

    sf::Vector2i findBestLocation(int mapWidth, int mapHeight,
                                  const std::vector<float> &suitabilityMap);
    sf::Vector2i findRandomSuitableLocation(int mapWidth, int mapHeight,
                                            const std::vector<float> &suitabilityMap);

private:
    LoadingState& _loadingState;
    WorldGenerationSystem& _worldGenerationSystem;
    EntityFactory& _entityFactory;
    Renderer& _renderer;
    PerformanceMonitor& _performanceMonitor;

    PlacementWeights _weights;
    SuitabilityMaps _suitabilityMaps;
    std::vector<sf::Vector2i> _placedCities;
    std::vector<TerrainType> _terrainCache;
    std::vector<int> _distanceToNearestCity;

    FastNoiseLite _noise;

    float _timeSinceLastCity = 0.0f;
    float _minSpawnInterval = 15.0f;
    float _maxSpawnInterval = 180.0f; // 3 minutes
    float _currentSpawnInterval;
    int _maxCities = 50;
    bool _initialPlacementDone = false;

    std::mt19937 _rng;
};