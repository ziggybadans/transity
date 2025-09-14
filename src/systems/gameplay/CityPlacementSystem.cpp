#include "CityPlacementSystem.h"
#include "core/ServiceLocator.h"
#include "ecs/EntityFactory.h"
#include "systems/world/WorldGenerationSystem.h"
#include "Logger.h"
#include "components/WorldComponents.h"
#include "core/PerfTimer.h"
#include "systems/rendering/TerrainRenderSystem.h"
#include "render/Renderer.h" 
#include "Constants.h"

#include <vector>
#include <algorithm>
#include <random>
#include <queue>

CityPlacementSystem::CityPlacementSystem(ServiceLocator &serviceLocator)
    : _serviceLocator(serviceLocator) {
    LOG_DEBUG("CityPlacementSystem", "CityPlacementSystem created.");
    _noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    _noise.SetFrequency(0.005f);
    _noise.SetSeed(std::random_device{}());
}

CityPlacementSystem::~CityPlacementSystem() {
    LOG_DEBUG("CityPlacementSystem", "CityPlacementSystem destroyed.");
}

const SuitabilityMaps &CityPlacementSystem::getSuitabilityMaps() const {
    return _suitabilityMaps;
}

void CityPlacementSystem::init() {
    placeCities(Constants::NUM_CITIES_TO_GENERATE);
}

void CityPlacementSystem::placeCities(int numberOfCities) {
    PerfTimer timer("CityPlacementSystem::placeCities", _serviceLocator, PerfTimer::Purpose::Log);
    LOG_INFO("CityPlacementSystem", "Starting city placement for %d cities...", numberOfCities);

    _serviceLocator.loadingState.message = "Analysing terrain for city placement...";
    _serviceLocator.loadingState.progress = 0.3f;

    auto &worldGenSystem = _serviceLocator.worldGenerationSystem;
    auto &entityFactory = _serviceLocator.entityFactory;
    const auto &worldGrid = worldGenSystem.getParams();
    const int mapWidth = worldGrid.worldDimensionsInChunks.x * worldGrid.chunkDimensionsInCells.x;
    const int mapHeight = worldGrid.worldDimensionsInChunks.y * worldGrid.chunkDimensionsInCells.y;
    const float cellSize = worldGrid.cellSize;

    precomputeTerrainCache(mapWidth, mapHeight);

    _serviceLocator.loadingState.message = "Calculating suitability maps...";
    _serviceLocator.loadingState.progress = 0.4f;

    _suitabilityMaps.water.resize(mapWidth * mapHeight, 0.0f);
    _suitabilityMaps.expandability.resize(mapWidth * mapHeight, 0.0f);
    _suitabilityMaps.cityProximity.resize(mapWidth * mapHeight, 1.0f);
    _suitabilityMaps.final.resize(mapWidth * mapHeight, 0.0f);
    _suitabilityMaps.noise.resize(mapWidth * mapHeight, 0.0f);

    _distanceToNearestCity.assign(mapWidth * mapHeight, std::numeric_limits<int>::max());
    _suitabilityMaps.cityProximity.assign(mapWidth * mapHeight, 1.0f);

    // Calculate and normalize static maps once
    {
        PerfTimer timer("calculateWaterSuitability", _serviceLocator, PerfTimer::Purpose::Log);
        calculateWaterSuitability(mapWidth, mapHeight, _suitabilityMaps.water);
        normalizeMap(_suitabilityMaps.water);
    }
    {
        PerfTimer timer("calculateExpandabilitySuitability", _serviceLocator, PerfTimer::Purpose::Log);
        calculateExpandabilitySuitability(mapWidth, mapHeight, _suitabilityMaps.expandability);
        normalizeMap(_suitabilityMaps.expandability);
    }
    {
        PerfTimer timer("calculateNoiseSuitability", _serviceLocator, PerfTimer::Purpose::Log);
        calculateNoiseSuitability(mapWidth, mapHeight, _suitabilityMaps.noise);
        normalizeMap(_suitabilityMaps.noise);
    }

    _serviceLocator.loadingState.message = "Placing cities...";

    for (int i = 0; i < numberOfCities; ++i) {
        PerfTimer loopTimer("CityPlacementLoop iteration " + std::to_string(i), _serviceLocator, PerfTimer::Purpose::Log);

        float cityProgress = static_cast<float>(i) / numberOfCities;
        _serviceLocator.loadingState.progress = 0.6f + (cityProgress * 0.4f);

        if (i > 0) {
            const auto &lastCity = _placedCities.back();
            {
                PerfTimer timer("updateDistanceMap", _serviceLocator, PerfTimer::Purpose::Log);
                updateDistanceMap(lastCity, mapWidth, mapHeight);
            }
            {
                PerfTimer timer ("calculateProximitySuitability", _serviceLocator, PerfTimer::Purpose::Log);
                calculateProximitySuitability(mapWidth, mapHeight, _suitabilityMaps.cityProximity);
                normalizeMap(_suitabilityMaps.cityProximity);
            }
        }
        
        {
            PerfTimer timer("combineSuitabilityMaps", _serviceLocator, PerfTimer::Purpose::Log);
            combineSuitabilityMaps(mapWidth, mapHeight, _suitabilityMaps, _weights, _suitabilityMaps.final);
        }

        sf::Vector2i bestLocation = findBestLocation(mapWidth, mapHeight, _suitabilityMaps.final);

        if (bestLocation.x == -1) {
            LOG_WARN("CityPlacementSystem", "No suitable location found for city %d. Halting.", i + 1);
            break;
        }

        entityFactory.createEntity("city", {bestLocation.x * cellSize + cellSize / 2.0f, bestLocation.y * cellSize + cellSize / 2.0f});
        _placedCities.push_back(bestLocation);
        LOG_DEBUG("CityPlacementSystem", "Placed city %d at (%d, %d)", i + 1, bestLocation.x, bestLocation.y);
    }

    LOG_INFO("CityPlacementSystem", "Finished city placement.");
    _serviceLocator.renderer.getTerrainRenderSystem().setSuitabilityMapData(&_suitabilityMaps, &_terrainCache, worldGrid);
}

void CityPlacementSystem::updateDistanceMap(const sf::Vector2i &newCity, int mapWidth, int mapHeight) {
    std::queue<sf::Vector2i> q;

    if (_distanceToNearestCity[newCity.y * mapWidth + newCity.x] > 0) {
        q.push(newCity);
        _distanceToNearestCity[newCity.y * mapWidth + newCity.x] = 0;
    } else {
        return; // Already processed
    }

    int dx[] = {0, 0, 1, -1, 1, 1, -1, -1};
    int dy[] = {1, -1, 0, 0, 1, -1, 1, -1};

    while (!q.empty()) {
        sf::Vector2i curr = q.front();
        q.pop();

        int d = _distanceToNearestCity[curr.y * mapWidth + curr.x];

        for (int i = 0; i < 8; ++i) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            if (nx >= 0 && nx < mapWidth && ny >= 0 && ny < mapHeight) {
                int newDist = d + 1;
                if (newDist < _distanceToNearestCity[ny * mapWidth + nx]) {
                    _distanceToNearestCity[ny * mapWidth + nx] = newDist;
                    q.push({nx, ny});
                }
            }
        }
    }
}

void CityPlacementSystem::calculateProximitySuitability(int mapWidth, int mapHeight, std::vector<float> &map) {
    const float idealDist = 80.0f; // The single threshold value

    for (int i = 0; i < mapWidth * mapHeight; ++i) {
        if (_distanceToNearestCity[i] == std::numeric_limits<int>::max()) {
            map[i] = 1.0f; // No cities nearby yet, max suitability
            continue;
        }

        float d = static_cast<float>(_distanceToNearestCity[i]);
        
        // Create a triangular function that peaks at idealDist
        // and is 0 at distance 0 and 2*idealDist.
        float dist_from_ideal = std::abs(d - idealDist);
        float score = 1.0f - dist_from_ideal / idealDist;
        score = std::max(0.0f, score); // Clamp score to be non-negative

        // Apply a smoothstep function for a nicer curve
        float smoothScore = score * score * (3.0f - 2.0f * score);
        map[i] = smoothScore;
    }
}

void CityPlacementSystem::precomputeTerrainCache(int mapWidth, int mapHeight) {
    PerfTimer timer("CityPlacementSystem::precomputeTerrainCache", _serviceLocator, PerfTimer::Purpose::Log);
    auto &worldGenSystem = _serviceLocator.worldGenerationSystem;
    const float cellSize = worldGenSystem.getParams().cellSize;

    _terrainCache.resize(mapWidth * mapHeight);
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            _terrainCache[y * mapWidth + x] = worldGenSystem.getTerrainTypeAt(x * cellSize, y * cellSize);
        }
    }
}

void CityPlacementSystem::calculateWaterSuitability(int mapWidth, int mapHeight, std::vector<float> &map) {
    const int maxDist = 60; // Max distance in cells to consider for water bonus

    std::vector<int> dist(mapWidth * mapHeight, -1);
    std::queue<sf::Vector2i> q;

    // Initialize queue with all water cells with cache
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            if (_terrainCache[y * mapWidth + x] == TerrainType::WATER) {
                q.push({x, y});
                dist[y * mapWidth + x] = 0;
            }
        }
    }

    // BFS to calculate distance from water
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

    // Assign suitability based on distance
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
            
            //map[y * mapWidth + x] = static_cast<float>(sum);
            float area = static_cast<float>((x2 - x1 + 1) * (y2 - y1 + 1));
            float landRatio = static_cast<float>(sum) / area;
            map[y * mapWidth + x] = landRatio * landRatio; // Square to favor higher ratios
        }
    }
}

void CityPlacementSystem::combineSuitabilityMaps(int mapWidth, int mapHeight, const SuitabilityMaps &maps,
                                                 const PlacementWeights &weights, std::vector<float> &finalMap) {
    for (int i = 0; i < mapWidth * mapHeight; ++i) {
        if (_terrainCache[i] != TerrainType::LAND) {
            finalMap[i] = 0;
            continue;
        }

        finalMap[i] = (maps.water[i] * weights.waterAccess) +
                      (maps.expandability[i] * weights.landExpandability) +
                      (maps.cityProximity[i] * weights.cityProximity) +
                      (maps.noise[i] * weights.randomness);
    }
}

void CityPlacementSystem::normalizeMap(std::vector<float> &map) {
    float minVal = std::numeric_limits<float>::max();
    float maxVal = std::numeric_limits<float>::min();

    for (float val : map) {
        if (val > 0) { // Only consider positive values for normalization
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

sf::Vector2i CityPlacementSystem::findBestLocation(int mapWidth, int mapHeight, const std::vector<float> &suitabilityMap) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disX(0, mapWidth - 1);
    std::uniform_int_distribution<> disY(0, mapHeight - 1);

    const int numSamples = 5000;
    const int numTopCandidates = 50;
    std::vector<std::pair<float, sf::Vector2i>> candidates;
    candidates.reserve(numSamples);

    for (int i = 0; i < numSamples; ++i) {
        int x = disX(gen);
        int y = disY(gen);
        
        float suitability = suitabilityMap[y * mapWidth + x];
        if (suitability > 0) { // Only consider valid locations
            candidates.push_back({suitability, {x, y}});
        }
    }

    if (candidates.empty()) {
        return {-1, -1};
    }

    // Sort candidates by suitability in descending order
    std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    // Pick one of the top N candidates
    int topN = std::min((int)candidates.size(), numTopCandidates);
    std::uniform_int_distribution<> disTop(0, topN - 1);
    
    return candidates[disTop(gen)].second;
}

void CityPlacementSystem::calculateNoiseSuitability(int mapWidth, int mapHeight, std::vector<float> &map) {
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            // Get noise value, which is in the range [-1, 1]
            float noiseVal = _noise.GetNoise((float)x, (float)y);
            // Map it to [0, 1]
            map[y * mapWidth + x] = (noiseVal + 1.0f) / 2.0f;
        }
    }
}