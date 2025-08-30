#include "CityPlacementSystem.h"
#include "core/ServiceLocator.h"
#include "ecs/EntityFactory.h"
#include "systems/world/WorldGenerationSystem.h"
#include "Logger.h"
#include "components/WorldComponents.h"

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
    calculateWaterSuitability(mapWidth, mapHeight, maps.water);
    calculateExpandabilitySuitability(mapWidth, mapHeight, maps.expandability);

    for (int i = 0; i < numberOfCities; ++i) {
        // Update dynamic maps and combine
        updateCityProximitySuitability(mapWidth, mapHeight, maps.cityProximity);
        combineSuitabilityMaps(mapWidth, mapHeight, maps, _weights, maps.final);

        sf::Vector2i bestLocation = findBestLocation(mapWidth, mapHeight, maps.final);

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
    const int radius = 15; // Radius to check for open land

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            if (worldGenSystem.getTerrainTypeAt(x * cellSize, y * cellSize) != TerrainType::LAND) {
                map[y * mapWidth + x] = 0;
                continue;
            }

            int landCount = 0;
            for (int ny = -radius; ny <= radius; ++ny) {
                for (int nx = -radius; nx <= radius; ++nx) {
                    int checkX = x + nx;
                    int checkY = y + ny;
                    if (checkX >= 0 && checkX < mapWidth && checkY >= 0 && checkY < mapHeight) {
                        if (worldGenSystem.getTerrainTypeAt(checkX * cellSize, checkY * cellSize) == TerrainType::LAND) {
                            landCount++;
                        }
                    }
                }
            }
            map[y * mapWidth + x] = static_cast<float>(landCount);
        }
    }
}

void CityPlacementSystem::updateCityProximitySuitability(int mapWidth, int mapHeight, std::vector<float> &map) {
    if (_placedCities.empty()) return;

    const float idealDist = 80.0f; // Ideal distance from the nearest city
    const float falloff = 0.01f;   // How quickly the score falls off from the ideal

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            float min_d = std::numeric_limits<float>::max();
            for (const auto &cityPos : _placedCities) {
                float d = std::sqrt(std::pow(x - cityPos.x, 2) + std::pow(y - cityPos.y, 2));
                if (d < min_d) {
                    min_d = d;
                }
            }
            float score = 1.0f - std::abs(min_d - idealDist) * falloff;
            map[y * mapWidth + x] = std::max(0.0f, score);
        }
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
    std::vector<sf::Vector2i> bestLocations;

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            float suitability = suitabilityMap[y * mapWidth + x];
            if (suitability > maxSuitability) {
                maxSuitability = suitability;
                bestLocations.clear();
                bestLocations.push_back({x, y});
            } else if (suitability == maxSuitability && maxSuitability > 0) {
                bestLocations.push_back({x, y});
            }
        }
    }

    if (bestLocations.empty()) {
        return {-1, -1};
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, bestLocations.size() - 1);
    return bestLocations[dis(gen)];
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