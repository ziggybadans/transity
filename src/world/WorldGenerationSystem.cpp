#include "WorldGenerationSystem.h"
#include "../Logger.h"
#include "../core/Constants.h"
#include "../core/PerfTimer.h"

#include <cassert>
#include <algorithm>
#include <random>
#include <vector>

WorldGenerationSystem::WorldGenerationSystem(entt::registry &registry, EventBus &eventBus)
    : _registry(registry), _eventBus(eventBus) {
    LOG_INFO("WorldGenerationSystem", "System created.");

    _params.noiseLayers.push_back({"Continents", 1337, 0.005f, FastNoiseLite::NoiseType_Perlin,
                                   FastNoiseLite::FractalType_FBm, 3, 2.0f, 0.5f, 1.0f});
    _params.noiseLayers.push_back({"Mountains", 1338, 0.02f, FastNoiseLite::NoiseType_Perlin,
                                   FastNoiseLite::FractalType_FBm, 6, 2.0f, 0.5f, 0.4f});
    _params.noiseLayers.push_back({"Erosion", 1339, 0.08f, FastNoiseLite::NoiseType_Cellular,
                                   FastNoiseLite::FractalType_None, 1, 2.0f, 0.5f, 0.15f});

    configureNoise();
}

WorldGenerationSystem::~WorldGenerationSystem() {}

void WorldGenerationSystem::setParams(const WorldGenParams &params) {
    _params = params;
    for (const auto& layer : _params.noiseLayers) {
        assert(layer.frequency > 0.0f && "Noise frequency must be positive.");
        assert(layer.octaves > 0 && "Noise octaves must be positive.");
        assert(layer.weight >= 0.0f && "Noise weight must be non-negative.");
    }
    configureNoise();
}

void WorldGenerationSystem::configureNoise() {
    _noiseGenerators.clear();
    for (const auto &layer : _params.noiseLayers) {
        FastNoiseLite noise;
        noise.SetSeed(layer.seed);
        noise.SetFrequency(layer.frequency);
        noise.SetNoiseType(layer.noiseType);
        noise.SetFractalType(layer.fractalType);
        noise.SetFractalOctaves(layer.octaves);
        noise.SetFractalLacunarity(layer.lacunarity);
        noise.SetFractalGain(layer.gain);
        _noiseGenerators.push_back(noise);
    }

    _coastlineDistortion.SetSeed(_params.noiseLayers.empty() ? 1337
                                                             : _params.noiseLayers[0].seed + 2);
    _coastlineDistortion.SetFrequency(
        _params.noiseLayers.empty() ? 0.02f : _params.noiseLayers[0].frequency * 4.0f);
    _coastlineDistortion.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    generateContinentShape();
}

void WorldGenerationSystem::generateContinentShape() {
    _params.continentShape.clear();
    sf::Vector2f worldSize = getWorldSize();
    sf::Vector2f center = worldSize / 2.0f;
    float radius = std::min(worldSize.x, worldSize.y) / 3.0f;
    int numPoints = 128;
    _params.continentShape.reserve(numPoints);

    FastNoiseLite shapeNoise;
    shapeNoise.SetSeed(_params.noiseLayers.empty() ? 1337 : _params.noiseLayers[0].seed + 1);
    shapeNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    shapeNoise.SetFrequency(2.0f);

    for (int i = 0; i < numPoints; ++i) {
        float angle = (float) i / (float) numPoints * 2.0f * 3.14159f;
        float distortedRadius = radius + shapeNoise.GetNoise((float) i, 0.0f) * radius * 0.4f;

        float x = center.x + distortedRadius * cos(angle);
        float y = center.y + distortedRadius * sin(angle);
        _params.continentShape.push_back({x, y});
    }
}

const WorldGridComponent &WorldGenerationSystem::getWorldGridSettings() const {
    auto view = _registry.view<WorldGridComponent>();
    if (view.empty()) {
        throw std::runtime_error("No WorldGridComponent found in the registry.");
    }
    return view.get<WorldGridComponent>(view.front());
}

sf::Vector2f WorldGenerationSystem::getWorldSize() {
    try {
        const WorldGridComponent &worldGrid = getWorldGridSettings();
        float worldWidth = static_cast<float>(worldGrid.worldDimensionsInChunks.x
                                              * worldGrid.chunkDimensionsInCells.x)
                           * worldGrid.cellSize;
        float worldHeight = static_cast<float>(worldGrid.worldDimensionsInChunks.y
                                               * worldGrid.chunkDimensionsInCells.y)
                            * worldGrid.cellSize;
        return {worldWidth, worldHeight};
    } catch (const std::runtime_error &e) {
        LOG_ERROR("WorldGenerationSystem", "Cannot get world size: %s", e.what());
        return {0.0f, 0.0f};
    }
}

GeneratedChunkData WorldGenerationSystem::generateChunkData(const sf::Vector2i& chunkGridPosition) const {
    PerfTimer timer("generateChunkData");
    
    const auto &worldGrid = getWorldGridSettings();
    sf::Vector2f worldSize = {
        static_cast<float>(worldGrid.worldDimensionsInChunks.x * worldGrid.chunkDimensionsInCells.x)
            * worldGrid.cellSize,
        static_cast<float>(worldGrid.worldDimensionsInChunks.y * worldGrid.chunkDimensionsInCells.y)
            * worldGrid.cellSize};
    sf::Vector2f center = worldSize / 2.0f;

    const int chunkCellSizeX = static_cast<int>(worldGrid.chunkDimensionsInCells.x);
    const int chunkCellSizeY = static_cast<int>(worldGrid.chunkDimensionsInCells.y);
    const int totalCells = chunkCellSizeX * chunkCellSizeY;

    GeneratedChunkData chunkData;
    chunkData.chunkGridPosition = chunkGridPosition;
    chunkData.cells.resize(totalCells);
    chunkData.noiseValues.resize(totalCells);
    chunkData.rawNoiseValues.resize(totalCells);

    auto processCell = [&](int x, int y, bool distort) {
        int cellIndex = y * chunkCellSizeX + x;
        float worldX = static_cast<float>((chunkGridPosition.x * chunkCellSizeX) + x)
                       * worldGrid.cellSize;
        float worldY = static_cast<float>((chunkGridPosition.y * chunkCellSizeY) + y)
                       * worldGrid.cellSize;

        float dx = center.x - worldX;
        float dy = center.y - worldY;
        float distance = std::sqrt(dx * dx + dy * dy);

        float maxDistance = std::min(worldSize.x, worldSize.y) / 2.5f;
        float falloff = 1.0f - std::min(1.0f, distance / maxDistance);

        float noiseX = static_cast<float>((chunkGridPosition.x * chunkCellSizeX) + x);
        float noiseY = static_cast<float>((chunkGridPosition.y * chunkCellSizeY) + y);

        float combinedNoise = 0.0f;
        float totalWeight = 0.0f;

        for (size_t i = 0; i < _noiseGenerators.size(); ++i) {
            float noiseValue = _noiseGenerators[i].GetNoise(noiseX, noiseY);
            noiseValue = (noiseValue + 1.0f) / 2.0f;
            combinedNoise += noiseValue * _params.noiseLayers[i].weight;
            totalWeight += _params.noiseLayers[i].weight;
        }

        if (totalWeight > 0) {
            combinedNoise /= totalWeight;
        }

        chunkData.rawNoiseValues[cellIndex] = combinedNoise;
        float finalValue = combinedNoise * falloff;

        float distortedLandThreshold = _params.landThreshold;
        if (distort) {
            float distortion = _coastlineDistortion.GetNoise(noiseX, noiseY)
                               * _params.coastlineDistortionStrength;
            distortedLandThreshold += distortion;
        }

        chunkData.noiseValues[cellIndex] = finalValue;
        chunkData.cells[cellIndex] =
            (finalValue > distortedLandThreshold) ? TerrainType::LAND : TerrainType::WATER;
    };

    if (_params.distortCoastline) {
        for (int y = 0; y < chunkCellSizeY; ++y) {
            for (int x = 0; x < chunkCellSizeX; ++x) {
                processCell(x, y, true);
            }
        }
    } else {
        for (int y = 0; y < chunkCellSizeY; ++y) {
            for (int x = 0; x < chunkCellSizeX; ++x) {
                processCell(x, y, false);
            }
        }
    }

    return chunkData;
}

void WorldGenerationSystem::regenerate(const WorldGenParams &params) {
    setParams(params);
}