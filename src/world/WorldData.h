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

struct WorldGenParams {
    std::vector<NoiseLayer> noiseLayers;
    float landThreshold = 0.35f;
    bool distortCoastline = false;
    float coastlineDistortionStrength = 0.1f;
    std::vector<Point> continentShape;
};

struct SwapWorldStateEvent {};

struct GeneratedChunkData {
    sf::Vector2i chunkGridPosition;
    std::vector<TerrainType> cells;
    std::vector<float> noiseValues;
    std::vector<float> rawNoiseValues;
};