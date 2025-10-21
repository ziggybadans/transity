#pragma once

#include "FastNoiseLite.h"
#include "TerrainType.h"
#include <SFML/System/Vector2.hpp>
#include <vector>

struct Point {
    float x, y;
};

struct NoiseLayer {
    std::string name = "Layer";
    int seed = 1337;
    float frequency = 0.02f;
    FastNoiseLite::NoiseType noiseType = FastNoiseLite::NoiseType_Perlin;
    FastNoiseLite::FractalType fractalType = FastNoiseLite::FractalType_FBm;
    int octaves = 5;
    float lacunarity = 2.0f;
    float gain = 0.5f;
    float weight = 1.0f;
};

struct ElevationParams {
    float maxElevation = 200.0f;
    float elevationExponent = 1.0f;
};

struct WorldGenParams {
    std::vector<NoiseLayer> noiseLayers;
    float landThreshold = 0.35f;
    float coastlineDistortionStrength = 0.0f;
    std::vector<Point> continentShape;
    ElevationParams elevation;

    sf::Vector2i worldDimensionsInChunks = {100, 100};
    sf::Vector2i chunkDimensionsInCells = {32, 32};
    float cellSize = 16.0f;
};

struct SwapWorldStateEvent {};

struct GeneratedChunkData {
    sf::Vector2i chunkGridPosition;
    std::vector<TerrainType> cells;
    std::vector<float> elevations;
};
