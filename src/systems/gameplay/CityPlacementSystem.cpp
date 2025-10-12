#include "CityPlacementSystem.h"
#include "ecs/EntityFactory.h"
#include "systems/world/WorldGenerationSystem.h"
#include "Logger.h"
#include "core/PerfTimer.h"
#include "systems/rendering/TerrainRenderSystem.h"
#include "render/Renderer.h" 
#include "Constants.h"
#include "app/LoadingState.h"
#include "core/ThreadPool.h"
#include <vector>
#include <algorithm>
#include <random>
#include <queue>

CityPlacementSystem::CityPlacementSystem(LoadingState& loadingState, WorldGenerationSystem& worldGenerationSystem, EntityFactory& entityFactory, Renderer& renderer, PerformanceMonitor& performanceMonitor, ThreadPool& threadPool)
    : _loadingState(loadingState), 
      _worldGenerationSystem(worldGenerationSystem), 
      _entityFactory(entityFactory), 
      _renderer(renderer), 
      _performanceMonitor(performanceMonitor),
      _threadPool(threadPool),
      _rng(std::random_device{}()) // Seed the random number generator
{
    LOG_DEBUG("CityPlacementSystem", "CityPlacementSystem created.");
    _noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    _noise.SetFrequency(Constants::CITY_PLACEMENT_NOISE_FREQUENCY);
    _noise.SetSeed(std::random_device{}());

    // Set the first spawn interval
    std::uniform_real_distribution<float> dist(_minSpawnInterval, _maxSpawnInterval);
    _currentSpawnInterval = dist(_rng);
    determineNextCityType();
}

CityPlacementSystem::~CityPlacementSystem() {
    LOG_DEBUG("CityPlacementSystem", "CityPlacementSystem destroyed.");
}

const SuitabilityMaps &CityPlacementSystem::getSuitabilityMaps() const {
    std::lock_guard<std::mutex> lock(_mapUpdateMutex);
    return _suitabilityMaps;
}

CityPlacementDebugInfo CityPlacementSystem::getDebugInfo() const {
    CityPlacementDebugInfo info = _debugInfo;
    info.timeToNextPlacement = _currentSpawnInterval - _timeSinceLastCity;
    info.nextCityType = _nextCityType;
    info.lastPlacementSuccess = _lastPlacementSuccess;
    return info;
}

void CityPlacementSystem::init() {
    initialPlacement();
}

void CityPlacementSystem::update(sf::Time dt) {
    if (!_initialPlacementDone || _placedCities.size() >= _maxCities) {
        return;
    }

    _timeSinceLastCity += dt.asSeconds();

    if (_timeSinceLastCity >= _currentSpawnInterval) {
        _lastPlacementSuccess = placeNewCity();
        if (_lastPlacementSuccess) {
            // On success, set a new random interval for the next city
            std::uniform_real_distribution<float> dist(_minSpawnInterval, _maxSpawnInterval);
            _currentSpawnInterval = dist(_rng);
            LOG_INFO("CityPlacementSystem", "New city placed. Next attempt in %.2f seconds.", _currentSpawnInterval);
        } else {
            // On failure, log it. The timer will be reset below.
            LOG_INFO("CityPlacementSystem", "Placement attempt failed. Trying again in %.2f seconds.", _currentSpawnInterval);
        }
        // Reset the timer regardless of success or failure.
        _timeSinceLastCity = 0.0f;
        determineNextCityType();
    }

    _debugInfoUpdateTimer += dt.asSeconds();
    if (_debugInfoUpdateTimer >= _debugInfoUpdateInterval) {
        updateDebugInfo();
        _debugInfoUpdateTimer = 0.0f;
    }
}

void CityPlacementSystem::initialPlacement() {
    PerfTimer timer("CityPlacementSystem::initialPlacement", _performanceMonitor, PerfTimer::Purpose::Log);
    LOG_INFO("CityPlacementSystem", "Starting initial city placement...");

    const auto &worldGrid = _worldGenerationSystem.getParams();
    const int mapWidth = worldGrid.worldDimensionsInChunks.x * worldGrid.chunkDimensionsInCells.x;
    const int mapHeight = worldGrid.worldDimensionsInChunks.y * worldGrid.chunkDimensionsInCells.y;

    _loadingState.message = "Analyzing terrain...";
    _loadingState.progress = 0.3f;
    precomputeTerrainCache(mapWidth, mapHeight);

    initializeSuitabilityMaps(mapWidth, mapHeight);
    calculateBaseSuitabilityMaps(mapWidth, mapHeight);

    LOG_INFO("CityPlacementSystem", "Placing initial settlements...");
    _loadingState.message = "Placing initial settlements...";
    placeInitialCapitals(mapWidth, mapHeight);

    LOG_INFO("CityPlacementSystem", "Calculating initial town and suburb suitability maps...");
    calculateDependentSuitabilityMaps(mapWidth, mapHeight);

    LOG_INFO("CityPlacementSystem", "Finished initial city placement.");
    _renderer.getTerrainRenderSystem().setSuitabilityMapData(&_suitabilityMaps, &_terrainCache, worldGrid);
    _initialPlacementDone = true;
    _loadingState.progress = 1.0f;
    _loadingState.message = "Finalizing world...";
    updateDebugInfo();
}

bool CityPlacementSystem::placeNewCity() {
    PerfTimer timer("CityPlacementSystem::placeNewCity", _performanceMonitor, PerfTimer::Purpose::Log);

    const auto &worldGrid = _worldGenerationSystem.getParams();
    const int mapWidth = worldGrid.worldDimensionsInChunks.x * worldGrid.chunkDimensionsInCells.x;
    const int mapHeight = worldGrid.worldDimensionsInChunks.y * worldGrid.chunkDimensionsInCells.y;
    const float cellSize = worldGrid.cellSize;

    sf::Vector2i location = {-1, -1};
    {
        std::lock_guard<std::mutex> lock(_mapUpdateMutex);
        if (_nextCityType == CityType::TOWN) {
            location = findRandomSuitableLocation(mapWidth, mapHeight, _suitabilityMaps.townFinal);
        } else {
            location = findRandomSuitableLocation(mapWidth, mapHeight, _suitabilityMaps.suburbFinal);
        }
    }

    if (location.x == -1) {
        LOG_INFO("CityPlacementSystem", "Random location did not meet suitability threshold. Trying again later.");
        return false;
    }

    std::string cityName = (_nextCityType == CityType::TOWN ? "Town " : "Suburb ") + std::to_string(_placedCities.size() + 1);
    _entityFactory.createEntity("city", {location.x * cellSize + cellSize / 2.0f, location.y * cellSize + cellSize / 2.0f}, _nextCityType, cityName);
    PlacedCityInfo newCity = {location, _nextCityType};
    _placedCities.push_back(newCity);
    LOG_INFO("CityPlacementSystem", "Placed new city %d at (%d, %d)", _placedCities.size(), location.x, location.y);
    
    asyncUpdateMaps(newCity);

    return true;
}

void CityPlacementSystem::asyncUpdateMaps(PlacedCityInfo newCity) {
    _threadPool.enqueue([this, newCity] {
        PerfTimer timer("CityPlacementSystem::asyncUpdateMaps", _performanceMonitor, PerfTimer::Purpose::Log);
        std::lock_guard<std::mutex> lock(_mapUpdateMutex);

        const auto &worldGrid = _worldGenerationSystem.getParams();
        const int mapWidth = worldGrid.worldDimensionsInChunks.x * worldGrid.chunkDimensionsInCells.x;
        const int mapHeight = worldGrid.worldDimensionsInChunks.y * worldGrid.chunkDimensionsInCells.y;

        updateDistanceMaps(newCity, mapWidth, mapHeight);

        calculateCapitalProximitySuitability(mapWidth, mapHeight, _suitabilityMaps.cityProximity);
        normalizeMap(_suitabilityMaps.cityProximity);
        calculateSuburbProximitySuitability(mapWidth, mapHeight, _suitabilityMaps.suburbProximity);
        normalizeMap(_suitabilityMaps.suburbProximity);
        calculateTownProximitySuitability(mapWidth, mapHeight, _suitabilityMaps.townProximity);
        normalizeMap(_suitabilityMaps.townProximity);
        combineSuitabilityMaps(mapWidth, mapHeight, _weights);

        _renderer.getTerrainRenderSystem().setSuitabilityMapData(&_suitabilityMaps, &_terrainCache, worldGrid);
        LOG_DEBUG("CityPlacementSystem", "Async map update complete for city at (%d, %d).", newCity.position.x, newCity.position.y);
    });
}

void CityPlacementSystem::updateDistanceMaps(const PlacedCityInfo &newCity, int mapWidth, int mapHeight) {
    std::vector<int>* distanceMap = nullptr;

    if (newCity.type == CityType::CAPITAL) {
        distanceMap = &_distanceToNearestCapital;
    } else if (newCity.type == CityType::TOWN) {
        distanceMap = &_distanceToNearestTown;
    } else {
        return; // Suburbs don't influence placement of other cities
    }

    std::queue<sf::Vector2i> q;
    if ((*distanceMap)[newCity.position.y * mapWidth + newCity.position.x] > 0) {
        q.push(newCity.position);
        (*distanceMap)[newCity.position.y * mapWidth + newCity.position.x] = 0;
    } else {
        return; // Already processed
    }

    int dx[] = {0, 0, 1, -1, 1, 1, -1, -1};
    int dy[] = {1, -1, 0, 0, 1, -1, 1, -1};

    while (!q.empty()) {
        sf::Vector2i curr = q.front();
        q.pop();

        int d = (*distanceMap)[curr.y * mapWidth + curr.x];

        for (int i = 0; i < 8; ++i) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            if (nx >= 0 && nx < mapWidth && ny >= 0 && ny < mapHeight) {
                int newDist = d + 1;
                if (newDist < (*distanceMap)[ny * mapWidth + nx]) {
                    (*distanceMap)[ny * mapWidth + nx] = newDist;
                    q.push({nx, ny});
                }
            }
        }
    }
}

void CityPlacementSystem::calculateCapitalProximitySuitability(int mapWidth, int mapHeight, std::vector<float> &map) {
    const float idealDist = Constants::CITY_PROXIMITY_IDEAL_DISTANCE;

    for (int i = 0; i < mapWidth * mapHeight; ++i) {
        if (_distanceToNearestCapital[i] == std::numeric_limits<int>::max()) {
            map[i] = 1.0f;
            continue;
        }

        float d = static_cast<float>(_distanceToNearestCapital[i]);
        float dist_from_ideal = std::abs(d - idealDist);
        float score = std::max(0.0f, 1.0f - dist_from_ideal / idealDist);
        map[i] = score * score * (3.0f - 2.0f * score);
    }
}

void CityPlacementSystem::calculateSuburbProximitySuitability(int mapWidth, int mapHeight, std::vector<float> &map) {
    const float capitalRange = Constants::SUBURB_PROXIMITY_RANGE_CAPITAL;
    const float townRange = Constants::SUBURB_PROXIMITY_RANGE_TOWN;

    for (int i = 0; i < mapWidth * mapHeight; ++i) {
        float distToCapital = static_cast<float>(_distanceToNearestCapital[i]);
        float distToTown = static_cast<float>(_distanceToNearestTown[i]);

        float score = 0.0f;
        if (distToCapital < capitalRange) {
            score = std::max(score, 1.0f - distToCapital / capitalRange);
        }
        if (distToTown < townRange) {
            score = std::max(score, 1.0f - distToTown / townRange);
        }
        map[i] = score;
    }
}

void CityPlacementSystem::calculateTownProximitySuitability(int mapWidth, int mapHeight, std::vector<float> &map) {
    const float min_dist = Constants::TOWN_PROXIMITY_MIN_DISTANCE;
    const float max_dist = Constants::TOWN_PROXIMITY_MAX_DISTANCE;

    for (int i = 0; i < mapWidth * mapHeight; ++i) {
        float distToCapital = static_cast<float>(_distanceToNearestCapital[i]);
        float distToTown = static_cast<float>(_distanceToNearestTown[i]);
        float minDist = std::min(distToCapital, distToTown);

        if (minDist < min_dist) {
            map[i] = 0.0f;
        } else if (minDist > max_dist) {
            map[i] = 1.0f;
        } else {
            map[i] = (minDist - min_dist) / (max_dist - min_dist);
        }
    }
}

void CityPlacementSystem::precomputeTerrainCache(int mapWidth, int mapHeight) {
    PerfTimer timer("CityPlacementSystem::precomputeTerrainCache", _performanceMonitor, PerfTimer::Purpose::Log);
    const float cellSize = _worldGenerationSystem.getParams().cellSize;

    _terrainCache.resize(mapWidth * mapHeight);
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            _terrainCache[y * mapWidth + x] = _worldGenerationSystem.getTerrainTypeAt(x * cellSize, y * cellSize);
        }
    }
}

void CityPlacementSystem::calculateWaterSuitability(int mapWidth, int mapHeight, std::vector<float> &map) {
    const int maxDist = Constants::WATER_SUITABILITY_MAX_DISTANCE;

    std::vector<int> dist(mapWidth * mapHeight, -1);
    std::queue<sf::Vector2i> q;

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            if (_terrainCache[y * mapWidth + x] == TerrainType::WATER) {
                q.push({x, y});
                dist[y * mapWidth + x] = 0;
            }
        }
    }

    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    while (!q.empty()) {
        sf::Vector2i curr = q.front();
        q.pop();

        if (dist[curr.y * mapWidth + curr.x] >= maxDist) continue;

        for (int i = 0; i < 4; ++i) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            if (nx >= 0 && nx < mapWidth && ny >= 0 && ny < mapHeight && dist[ny * mapWidth + nx] == -1) {
                dist[ny * mapWidth + nx] = dist[curr.y * mapWidth + curr.x] + 1;
                q.push({nx, ny});
            }
        }
    }

    for (int i = 0; i < mapWidth * mapHeight; ++i) {
        if (dist[i] != -1) {
            map[i] = std::max(0.0f, 1.0f - static_cast<float>(dist[i]) / maxDist);
        }
    }
}

void CityPlacementSystem::calculateExpandabilitySuitability(int mapWidth, int mapHeight, std::vector<float> &map) {
    const int radius = 20;

    std::vector<int> landMap(mapWidth * mapHeight, 0);
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            if (_terrainCache[y * mapWidth + x] == TerrainType::LAND) {
                landMap[y * mapWidth + x] = 1;
            }
        }
    }

    std::vector<long long> sat(mapWidth * mapHeight, 0);
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            long long val = landMap[y * mapWidth + x];
            if (x > 0) val += sat[y * mapWidth + (x - 1)];
            if (y > 0) val += sat[(y - 1) * mapWidth + x];
            if (x > 0 && y > 0) val -= sat[(y - 1) * mapWidth + (x - 1)];
            sat[y * mapWidth + x] = val;
        }
    }

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            int x1 = std::max(0, x - radius);
            int y1 = std::max(0, y - radius);
            int x2 = std::min(mapWidth - 1, x + radius);
            int y2 = std::min(mapHeight - 1, y + radius);

            long long sum = sat[y2 * mapWidth + x2];
            if (x1 > 0) sum -= sat[y2 * mapWidth + (x1 - 1)];
            if (y1 > 0) sum -= sat[(y1 - 1) * mapWidth + x2];
            if (x1 > 0 && y1 > 0) sum += sat[(y1 - 1) * mapWidth + (x1 - 1)];
            
            float area = static_cast<float>((x2 - x1 + 1) * (y2 - y1 + 1));
            float landRatio = static_cast<float>(sum) / area;
            map[y * mapWidth + x] = landRatio * landRatio;
        }
    }
}

void CityPlacementSystem::combineSuitabilityMaps(int mapWidth, int mapHeight, const PlacementWeights &weights) {
    for (int i = 0; i < mapWidth * mapHeight; ++i) {
        if (_terrainCache[i] != TerrainType::LAND) {
            _suitabilityMaps.final[i] = 0;
            _suitabilityMaps.townFinal[i] = 0;
            _suitabilityMaps.suburbFinal[i] = 0;
            continue;
        }

        float baseSuitability = (_suitabilityMaps.water[i] * weights.waterAccess) +
                                (_suitabilityMaps.expandability[i] * weights.landExpandability) +
                                (_suitabilityMaps.noise[i] * weights.randomness);

        _suitabilityMaps.final[i] = baseSuitability + (_suitabilityMaps.cityProximity[i] * weights.cityProximity);
        _suitabilityMaps.townFinal[i] = baseSuitability + (_suitabilityMaps.townProximity[i] * weights.cityProximity);
        _suitabilityMaps.suburbFinal[i] = baseSuitability + (_suitabilityMaps.suburbProximity[i] * weights.cityProximity);
    }
}

void CityPlacementSystem::normalizeMap(std::vector<float> &map) {
    float minVal = std::numeric_limits<float>::max();
    float maxVal = std::numeric_limits<float>::min();

    for (float val : map) {
        if (val > 0) {
            minVal = std::min(minVal, val);
            maxVal = std::max(maxVal, val);
        }
    }

    if (maxVal > minVal) {
        for (float &val : map) {
            if (val > 0) {
                val = (val - minVal) / (maxVal - minVal);
            }
        }
    }
}

sf::Vector2i CityPlacementSystem::findRandomSuitableLocation(int mapWidth, int mapHeight, const std::vector<float>& suitabilityMap) {
    const int maxAttempts = Constants::FIND_RANDOM_CITY_LOCATION_ATTEMPTS;
    std::uniform_int_distribution<> disX(0, mapWidth - 1);
    std::uniform_int_distribution<> disY(0, mapHeight - 1);
    std::uniform_real_distribution<float> threshold_dist(Constants::FIND_RANDOM_CITY_MIN_SUITABILITY, Constants::FIND_RANDOM_CITY_MAX_SUITABILITY);

    for (int i = 0; i < maxAttempts; ++i) {
        int x = disX(_rng);
        int y = disY(_rng);
        float threshold = threshold_dist(_rng);
        float suitability = suitabilityMap[y * mapWidth + x];

        if (suitability >= threshold) {
            LOG_DEBUG("CityPlacementSystem", "Found suitable random location at (%d, %d) on attempt %d with suitability %.2f (threshold %.2f)", x, y, i + 1, suitability, threshold);
            return {x, y};
        }
    }

    return {-1, -1};
}

void CityPlacementSystem::calculateNoiseSuitability(int mapWidth, int mapHeight, std::vector<float> &map) {
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            float noiseVal = _noise.GetNoise((float)x, (float)y);
            map[y * mapWidth + x] = (noiseVal + 1.0f) / 2.0f;
        }
    }
}

sf::Vector2i CityPlacementSystem::findBestLocation(int mapWidth, int mapHeight, const std::vector<float> &suitabilityMap) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disX(0, mapWidth - 1);
    std::uniform_int_distribution<> disY(0, mapHeight - 1);

    const int numSamples = Constants::FIND_BEST_CITY_LOCATION_SAMPLES;
    const int numTopCandidates = Constants::FIND_BEST_CITY_LOCATION_TOP_CANDIDATES;
    std::vector<std::pair<float, sf::Vector2i>> candidates;
    candidates.reserve(numSamples);

    for (int i = 0; i < numSamples; ++i) {
        int x = disX(gen);
        int y = disY(gen);
        
        float suitability = suitabilityMap[y * mapWidth + x];
        if (suitability > 0) {
            candidates.push_back({suitability, {x, y}});
        }
    }

    if (candidates.empty()) {
        return {-1, -1};
    }

    std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    int topN = std::min((int)candidates.size(), numTopCandidates);
    std::uniform_int_distribution<> disTop(0, topN - 1);
    
    return candidates[disTop(gen)].second;
}

void CityPlacementSystem::determineNextCityType() {
    std::uniform_real_distribution<float> dist(0.0, 1.0);
    _nextCityType = (dist(_rng) < 0.5f) ? CityType::TOWN : CityType::SUBURB;
}

void CityPlacementSystem::updateDebugInfo() {
    std::lock_guard<std::mutex> lock(_mapUpdateMutex);
    int townSuitableCount = 0;
    int suburbSuitableCount = 0;
    int totalLandCells = 0;

    if (_suitabilityMaps.townFinal.empty() || _terrainCache.empty()) {
        _debugInfo.townSuitabilityPercentage = 0.0f;
        _debugInfo.suburbSuitabilityPercentage = 0.0f;
        return;
    }

    for (size_t i = 0; i < _suitabilityMaps.townFinal.size(); ++i) {
        if (_terrainCache[i] == TerrainType::LAND) {
            totalLandCells++;
            if (_suitabilityMaps.townFinal[i] >= Constants::FIND_RANDOM_CITY_MIN_SUITABILITY) {
                townSuitableCount++;
            }
            if (_suitabilityMaps.suburbFinal[i] >= Constants::FIND_RANDOM_CITY_MIN_SUITABILITY) {
                suburbSuitableCount++;
            }
        }
    }

    if (totalLandCells > 0) {
        _debugInfo.townSuitabilityPercentage = static_cast<float>(townSuitableCount) / totalLandCells * 100.0f;
        _debugInfo.suburbSuitabilityPercentage = static_cast<float>(suburbSuitableCount) / totalLandCells * 100.0f;
    } else {
        _debugInfo.townSuitabilityPercentage = 0.0f;
        _debugInfo.suburbSuitabilityPercentage = 0.0f;
    }
}

void CityPlacementSystem::initializeSuitabilityMaps(int mapWidth, int mapHeight) {
    _suitabilityMaps.water.resize(mapWidth * mapHeight, 0.0f);
    _suitabilityMaps.expandability.resize(mapWidth * mapHeight, 0.0f);
    _suitabilityMaps.cityProximity.resize(mapWidth * mapHeight, 1.0f);
    _suitabilityMaps.noise.resize(mapWidth * mapHeight, 0.0f);
    _suitabilityMaps.final.resize(mapWidth * mapHeight, 0.0f);
    _suitabilityMaps.townProximity.resize(mapWidth * mapHeight, 0.0f);
    _suitabilityMaps.suburbProximity.resize(mapWidth * mapHeight, 0.0f);
    _suitabilityMaps.townFinal.resize(mapWidth * mapHeight, 0.0f);
    _suitabilityMaps.suburbFinal.resize(mapWidth * mapHeight, 0.0f);

    _distanceToNearestCapital.assign(mapWidth * mapHeight, std::numeric_limits<int>::max());
    _distanceToNearestTown.assign(mapWidth * mapHeight, std::numeric_limits<int>::max());
}

void CityPlacementSystem::calculateBaseSuitabilityMaps(int mapWidth, int mapHeight) {
    _loadingState.message = "Assessing water access...";
    calculateWaterSuitability(mapWidth, mapHeight, _suitabilityMaps.water);
    normalizeMap(_suitabilityMaps.water);
    _loadingState.progress = 0.4f;

    _loadingState.message = "Evaluating expansion potential...";
    calculateExpandabilitySuitability(mapWidth, mapHeight, _suitabilityMaps.expandability);
    normalizeMap(_suitabilityMaps.expandability);
    _loadingState.progress = 0.5f;

    _loadingState.message = "Adding environmental noise...";
    calculateNoiseSuitability(mapWidth, mapHeight, _suitabilityMaps.noise);
    normalizeMap(_suitabilityMaps.noise);
    _loadingState.progress = 0.6f;
}

void CityPlacementSystem::placeInitialCapitals(int mapWidth, int mapHeight) {
    const float cellSize = _worldGenerationSystem.getParams().cellSize;
    for (int i = 0; i < Constants::INITIAL_CITY_COUNT; ++i) {
        if (i > 0) {
            calculateCapitalProximitySuitability(mapWidth, mapHeight, _suitabilityMaps.cityProximity);
            normalizeMap(_suitabilityMaps.cityProximity);
        }
        
        combineSuitabilityMaps(mapWidth, mapHeight, _weights);
        sf::Vector2i bestLocation = findBestLocation(mapWidth, mapHeight, _suitabilityMaps.final);

        if (bestLocation.x == -1) {
            LOG_ERROR("CityPlacementSystem", "Failed to place initial city %d.", i + 1);
            continue;
        }

        std::string cityName = "City " + std::to_string(_placedCities.size() + 1);
        _entityFactory.createEntity("city", {bestLocation.x * cellSize + cellSize / 2.0f, bestLocation.y * cellSize + cellSize / 2.0f}, CityType::CAPITAL, cityName);
        PlacedCityInfo newCity = {bestLocation, CityType::CAPITAL};
        _placedCities.push_back(newCity);
        LOG_INFO("CityPlacementSystem", "Placed initial city %d at (%d, %d)", _placedCities.size(), bestLocation.x, bestLocation.y);
        
        updateDistanceMaps(newCity, mapWidth, mapHeight);
        _loadingState.progress = 0.7f + (i * 0.1f);
    }
}

void CityPlacementSystem::calculateDependentSuitabilityMaps(int mapWidth, int mapHeight) {
    calculateSuburbProximitySuitability(mapWidth, mapHeight, _suitabilityMaps.suburbProximity);
    normalizeMap(_suitabilityMaps.suburbProximity);
    calculateTownProximitySuitability(mapWidth, mapHeight, _suitabilityMaps.townProximity);
    normalizeMap(_suitabilityMaps.townProximity);
    combineSuitabilityMaps(mapWidth, mapHeight, _weights);
}