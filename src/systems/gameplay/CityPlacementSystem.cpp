#include "CityPlacementSystem.h"
#include "core/ServiceLocator.h"
#include "ecs/EntityFactory.h"
#include "systems/world/WorldGenerationSystem.h"
#include "Logger.h"
#include "components/WorldComponents.h"
#include "core/PerfTimer.h"

#include <vector>
#include <algorithm>
#include <random>
#include <queue>

CityPlacementSystem::CityPlacementSystem(ServiceLocator &serviceLocator)
    : _serviceLocator(serviceLocator) {
    LOG_DEBUG("CityPlacementSystem", "CityPlacementSystem created.");
}

CityPlacementSystem::~CityPlacementSystem() {
    LOG_DEBUG("CityPlacementSystem", "CityPlacementSystem destroyed.");
}

void CityPlacementSystem::update(sf::Time dt) {
    if (_hasRun) {
        return;
    }

    placeCities(10);
    _hasRun = true;
}

void CityPlacementSystem::placeCities(int numberOfCities) {
    PerfTimer timer("CityPlacementSystem::placeCities", _serviceLocator, PerfTimer::Purpose::Log);
    LOG_INFO("CityPlacementSystem", "Starting city placement for %d cities...", numberOfCities);

    auto &worldGenSystem = _serviceLocator.worldGenerationSystem;
    auto &entityFactory = _serviceLocator.entityFactory;
    const auto &worldGrid = worldGenSystem.getParams();
    const int mapWidth = worldGrid.worldDimensionsInChunks.x * worldGrid.chunkDimensionsInCells.x;
    const int mapHeight = worldGrid.worldDimensionsInChunks.y * worldGrid.chunkDimensionsInCells.y;
    const float cellSize = worldGrid.cellSize;

    SuitabilityMaps maps;
    maps.water.resize(mapWidth * mapHeight, 0.0f);
    maps.expandability.resize(mapWidth * mapHeight, 0.0f);
    maps.cityProximity.resize(mapWidth * mapHeight, 1.0f); // Start with a neutral score
    maps.final.resize(mapWidth * mapHeight, 0.0f);

    // Calculate static suitability maps once
    {
        PerfTimer timer("calculateWaterSuitability", _serviceLocator, PerfTimer::Purpose::Log);
        calculateWaterSuitability(mapWidth, mapHeight, maps.water);
    }
    {
        PerfTimer timer("calculateExpandabilitySuitability", _serviceLocator, PerfTimer::Purpose::Log);
        calculateExpandabilitySuitability(mapWidth, mapHeight, maps.expandability);
    }

    for (int i = 0; i < numberOfCities; ++i) {
        PerfTimer loopTimer("CityPlacementLoop iteration " + std::to_string(i), _serviceLocator, PerfTimer::Purpose::Log);
        
        // Update dynamic maps and combine
        {
            PerfTimer timer("updateCityProximitySuitability", _serviceLocator, PerfTimer::Purpose::Log);
            updateCityProximitySuitability(mapWidth, mapHeight, maps.cityProximity);
        }
        {
            PerfTimer timer("combineSuitabilityMaps", _serviceLocator, PerfTimer::Purpose::Log);
            combineSuitabilityMaps(mapWidth, mapHeight, maps, _weights, maps.final);
        }

        sf::Vector2i bestLocation;
        {
            PerfTimer timer("findBestLocation", _serviceLocator, PerfTimer::Purpose::Log);
            bestLocation = findBestLocation(mapWidth, mapHeight, maps.final);
        }

        if (bestLocation.x == -1) {
            LOG_WARN("CityPlacementSystem", "No suitable location found for city %d. Halting.", i + 1);
            break;
        }

        float worldX = bestLocation.x * cellSize + cellSize / 2.0f;
        float worldY = bestLocation.y * cellSize + cellSize / 2.0f;

        entityFactory.createEntity("city", {worldX, worldY});
        _placedCities.push_back(bestLocation);
        LOG_DEBUG("CityPlacementSystem", "Placed city %d at (%d, %d)", i + 1, bestLocation.x, bestLocation.y);

        // Make the area around the new city unsuitable for the next placement
        reduceSuitabilityAroundCity(bestLocation.x, bestLocation.y, mapWidth, mapHeight, maps.final);
    }

    LOG_INFO("CityPlacementSystem", "Finished city placement.");
}

void CityPlacementSystem::calculateWaterSuitability(int mapWidth, int mapHeight, std::vector<float> &map) {
    auto &worldGenSystem = _serviceLocator.worldGenerationSystem;
    const float cellSize = worldGenSystem.getParams().cellSize;
    const int maxDist = 20; // Max distance in cells to consider for water bonus

    std::vector<int> dist(mapWidth * mapHeight, -1);
    std::queue<sf::Vector2i> q;

    // Initialize queue with all water cells
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            if (worldGenSystem.getTerrainTypeAt(x * cellSize, y * cellSize) == TerrainType::WATER) {
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
    auto &worldGenSystem = _serviceLocator.worldGenerationSystem;
    const float cellSize = worldGenSystem.getParams().cellSize;
    const int radius = 15;

    std::vector<int> landMap(mapWidth * mapHeight, 0);
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            if (worldGenSystem.getTerrainTypeAt(x * cellSize, y * cellSize) == TerrainType::LAND) {
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
            
            map[y * mapWidth + x] = static_cast<float>(sum);
        }
    }
}

void CityPlacementSystem::updateCityProximitySuitability(int mapWidth, int mapHeight, std::vector<float> &map) {
    if (_placedCities.empty()) return;

    const float idealDist = 80.0f;
    const float falloff = 0.01f;

    std::vector<int> dist(mapWidth * mapHeight, -1);
    std::queue<sf::Vector2i> q;

    for (const auto &cityPos : _placedCities) {
        q.push(cityPos);
        dist[cityPos.y * mapWidth + cityPos.x] = 0;
    }

    int dx[] = {0, 0, 1, -1, 1, 1, -1, -1};
    int dy[] = {1, -1, 0, 0, 1, -1, 1, -1};

    while (!q.empty()) {
        sf::Vector2i curr = q.front();
        q.pop();

        for (int i = 0; i < 8; ++i) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            if (nx >= 0 && nx < mapWidth && ny >= 0 && ny < mapHeight && dist[ny * mapWidth + nx] == -1) {
                dist[ny * mapWidth + nx] = dist[curr.y * mapWidth + curr.x] + 1;
                q.push({nx, ny});
            }
        }
    }

    for (int i = 0; i < mapWidth * mapHeight; ++i) {
        float d = static_cast<float>(dist[i]);
        float score = 1.0f - std::abs(d - idealDist) * falloff;
        map[i] = std::max(0.0f, score);
    }
}

void CityPlacementSystem::combineSuitabilityMaps(int mapWidth, int mapHeight, const SuitabilityMaps &maps,
                                                 const PlacementWeights &weights, std::vector<float> &finalMap) {
    SuitabilityMaps normalizedMaps = maps;
    normalizeMap(normalizedMaps.water);
    normalizeMap(normalizedMaps.expandability);
    normalizeMap(normalizedMaps.cityProximity);

    auto &worldGenSystem = _serviceLocator.worldGenerationSystem;
    const float cellSize = worldGenSystem.getParams().cellSize;

    for (int i = 0; i < mapWidth * mapHeight; ++i) {
        // Ensure we only place on land
        if (worldGenSystem.getTerrainTypeAt((i % mapWidth) * cellSize, (i / mapWidth) * cellSize) != TerrainType::LAND) {
            finalMap[i] = 0;
            continue;
        }

        finalMap[i] = (normalizedMaps.water[i] * weights.waterAccess) +
                      (normalizedMaps.expandability[i] * weights.landExpandability) +
                      (normalizedMaps.cityProximity[i] * weights.cityProximity);
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
    float maxSuitability = 0.0f;
    sf::Vector2i bestLocation = {-1, -1};
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disX(0, mapWidth - 1);
    std::uniform_int_distribution<> disY(0, mapHeight - 1);

    const int numSamples = 5000;

    for (int i = 0; i < numSamples; ++i) {
        int x = disX(gen);
        int y = disY(gen);
        
        float suitability = suitabilityMap[y * mapWidth + x];
        if (suitability > maxSuitability) {
            maxSuitability = suitability;
            bestLocation = {x, y};
        }
    }

    return bestLocation;
}

void CityPlacementSystem::reduceSuitabilityAroundCity(int cityX, int cityY, int mapWidth, int mapHeight, std::vector<float> &suitabilityMap) {
    const int minDistanceCells = 30;

    for (int y = -minDistanceCells; y <= minDistanceCells; ++y) {
        for (int x = -minDistanceCells; x <= minDistanceCells; ++x) {
            int currentX = cityX + x;
            int currentY = cityY + y;

            if (currentX >= 0 && currentX < mapWidth && currentY >= 0 && currentY < mapHeight) {
                float distance = std::sqrt(x * x + y * y);
                if (distance < minDistanceCells) {
                    suitabilityMap[currentY * mapWidth + currentX] = 0.0f;
                }
            }
        }
    }
}