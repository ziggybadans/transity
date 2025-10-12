#pragma once

#include "ecs/ISystem.h"
#include "entt/entt.hpp"
#include "components/WorldComponents.h"
#include "components/GameLogicComponents.h"
#include "FastNoiseLite.h"
#include "Constants.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Time.hpp>
#include <vector>
#include <random>
#include <mutex>

struct LoadingState;
class WorldGenerationSystem;
class EntityFactory;
class Renderer;
class PerformanceMonitor;
class ThreadPool;

struct PlacementWeights {
    float waterAccess = Constants::SUITABILITY_WEIGHT_WATER;
    float landExpandability = Constants::SUITABILITY_WEIGHT_EXPANDABILITY;
    float cityProximity = Constants::SUITABILITY_WEIGHT_PROXIMITY;
    float randomness = Constants::SUITABILITY_WEIGHT_RANDOMNESS;
};

struct SuitabilityMaps {
    std::vector<float> water;
    std::vector<float> expandability;
    std::vector<float> cityProximity;
    std::vector<float> noise;
    std::vector<float> final;
    std::vector<float> townProximity;
    std::vector<float> suburbProximity;
    std::vector<float> townFinal;
    std::vector<float> suburbFinal;
};

struct PlacedCityInfo {
    sf::Vector2i position;
    CityType type;
};

struct CityPlacementDebugInfo {
    float timeToNextPlacement = 0.0f;
    CityType nextCityType = CityType::TOWN;
    bool lastPlacementSuccess = false;
    float townSuitabilityPercentage = 0.0f;
    float suburbSuitabilityPercentage = 0.0f;
};

class CityPlacementSystem : public ISystem, public IUpdatable {
public:
    explicit CityPlacementSystem(LoadingState& loadingState, WorldGenerationSystem& worldGenerationSystem, EntityFactory& entityFactory, Renderer& renderer, PerformanceMonitor& performanceMonitor, ThreadPool& threadPool);
    ~CityPlacementSystem() override;

    void init();
    void update(sf::Time dt) override;

    const SuitabilityMaps &getSuitabilityMaps() const;
    CityPlacementDebugInfo getDebugInfo() const;

private:
    void initialPlacement();
    bool placeNewCity();
    void asyncUpdateMaps(PlacedCityInfo newCity);
    
    void precomputeTerrainCache(int mapWidth, int mapHeight);

    sf::Vector2i findBestLocation(int mapWidth, int mapHeight,
                                  const std::vector<float> &suitabilityMap);
    sf::Vector2i findRandomSuitableLocation(int mapWidth, int mapHeight,
                                            const std::vector<float> &suitabilityMap);
    void determineNextCityType();
    void updateDebugInfo();

    void initializeSuitabilityMaps(int mapWidth, int mapHeight);
    void calculateBaseSuitabilityMaps(int mapWidth, int mapHeight);
    void placeInitialCapitals(int mapWidth, int mapHeight);
    void calculateDependentSuitabilityMaps(int mapWidth, int mapHeight);

    void calculateWaterSuitability(int mapWidth, int mapHeight, std::vector<float> &map);
    void calculateExpandabilitySuitability(int mapWidth, int mapHeight, std::vector<float> &map);
    void calculateNoiseSuitability(int mapWidth, int mapHeight, std::vector<float> &map);

    void updateDistanceMaps(const PlacedCityInfo &newCity, int mapWidth, int mapHeight);
    void calculateCapitalProximitySuitability(int mapWidth, int mapHeight, std::vector<float> &map);
    void calculateSuburbProximitySuitability(int mapWidth, int mapHeight, std::vector<float> &map);
    void calculateTownProximitySuitability(int mapWidth, int mapHeight, std::vector<float> &map);

    void combineSuitabilityMaps(int mapWidth, int mapHeight, const PlacementWeights &weights);
    void normalizeMap(std::vector<float> &map);

    LoadingState& _loadingState;
    WorldGenerationSystem& _worldGenerationSystem;
    EntityFactory& _entityFactory;
    Renderer& _renderer;
    PerformanceMonitor& _performanceMonitor;
    ThreadPool& _threadPool;

    PlacementWeights _weights;
    SuitabilityMaps _suitabilityMaps;
    std::vector<PlacedCityInfo> _placedCities;
    std::vector<TerrainType> _terrainCache;
    std::vector<int> _distanceToNearestCapital;
    std::vector<int> _distanceToNearestTown;

    FastNoiseLite _noise;

    float _timeSinceLastCity = 0.0f;
    float _currentSpawnInterval;
    float _minSpawnInterval = Constants::MIN_CITY_SPAWN_INTERVAL_S;
    float _maxSpawnInterval = Constants::MAX_CITY_SPAWN_INTERVAL_S;
    int _maxCities = Constants::MAX_CITIES;
    bool _initialPlacementDone = false;
    bool _lastPlacementSuccess = false;
    CityType _nextCityType = CityType::TOWN;

    CityPlacementDebugInfo _debugInfo;
    float _debugInfoUpdateTimer = 0.0f;
    const float _debugInfoUpdateInterval = 1.0f;

    std::mt19937 _rng;
    mutable std::mutex _mapUpdateMutex;
};