#include "CityPlacementSystem.h"
#include "core/ServiceLocator.h"
#include "ecs/EntityFactory.h"
#include "systems/world/WorldGenerationSystem.h"
#include "Logger.h"
#include "components/WorldComponents.h"

#include <vector>
#include <algorithm>
#include <random>

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
    LOG_DEBUG("CityPlacementSystem", "Placing %d cities...", numberOfCities);

    auto &worldGenSystem = _serviceLocator.worldGenerationSystem;
    auto &entityFactory = _serviceLocator.entityFactory;

    const auto &worldGrid = worldGenSystem.getParams();
    const int mapWidth = worldGrid.worldDimensionsInChunks.x * worldGrid.chunkDimensionsInCells.x;
    const int mapHeight = worldGrid.worldDimensionsInChunks.y * worldGrid.chunkDimensionsInCells.y;
    const float cellSize = worldGrid.cellSize;

    std::vector<float> suitabilityMap(mapWidth * mapHeight, 0.0f);

    // Step 1: Base suitability on terrain type
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            float worldX = x * worldGrid.cellSize;
            float worldY = y * worldGrid.cellSize;
            if (worldGenSystem.getTerrainTypeAt(worldX, worldY) == TerrainType::LAND) {
                suitabilityMap[y * mapWidth + x] = 1.0f; // Base score for land
            }
        }
    }

    // Step 2: Increase suitability for land near water
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            if (suitabilityMap[y * mapWidth + x] > 0) { // Is it land?
                bool isCoastal = false;
                // Check neighbors
                for (int ny = -1; ny <= 1; ++ny) {
                    for (int nx = -1; nx <= 1; ++nx) {
                        if (nx == 0 && ny == 0) continue;
                        int neighborX = x + nx;
                        int neighborY = y + ny;

                        if (neighborX >= 0 && neighborX < mapWidth && neighborY >= 0 && neighborY < mapHeight) {
                            float neighborWorldX = neighborX * worldGrid.cellSize;
                            float neighborWorldY = neighborY * worldGrid.cellSize;
                            if (worldGenSystem.getTerrainTypeAt(neighborWorldX, neighborWorldY) == TerrainType::WATER) {
                                isCoastal = true;
                                break;
                            }
                        }
                    }
                    if (isCoastal) break;
                }

                if (isCoastal) {
                    suitabilityMap[y * mapWidth + x] = 5.0f; // Higher score for coastal land
                }
            }
        }
    }
    
    // Step 3: Iteratively place cities
    for (int i = 0; i < numberOfCities; ++i) {
        sf::Vector2i bestLocation = findBestLocation(mapWidth, mapHeight, suitabilityMap);

        if (bestLocation.x == -1) {
            LOG_WARN("CityPlacementSystem", "No suitable location found for city %d. Halting placement.", i + 1);
            break;
        }

        float worldX = bestLocation.x * worldGrid.cellSize + worldGrid.cellSize / 2.0f;
        float worldY = bestLocation.y * worldGrid.cellSize + worldGrid.cellSize / 2.0f;

        entityFactory.createEntity("city", {worldX, worldY});
        LOG_DEBUG("CityPlacementSystem", "Placed city %d at (%.1f, %.1f)", i + 1, worldX, worldY);

        reduceSuitabilityAroundCity(bestLocation.x, bestLocation.y, mapWidth, mapHeight, suitabilityMap);
    }

    LOG_INFO("CityPlacementSystem", "Finished city placement.");
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

    LOG_DEBUG("CityPlacementSystem", "Max suitability found: %.2f", maxSuitability);
    LOG_DEBUG("CityPlacementSystem", "Found %d best locations.", bestLocations.size());

    if (bestLocations.empty()) {
        return {-1, -1}; // No suitable location found
    }

    // If there are multiple best locations, pick one at random
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, bestLocations.size() - 1);
    return bestLocations[dis(gen)];
}

void CityPlacementSystem::reduceSuitabilityAroundCity(int cityX, int cityY, int mapWidth, int mapHeight, std::vector<float> &suitabilityMap) {
    const int minDistanceCells = 30; // Minimum distance between cities in grid cells

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