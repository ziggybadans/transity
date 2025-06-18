#pragma once

#include "FastNoiseLite.h"

struct WorldGenParams {
    int seed = 1337;
    float frequency = 0.02f;
    FastNoiseLite::NoiseType noiseType = FastNoiseLite::NoiseType_Perlin;
    FastNoiseLite::FractalType fractalType = FastNoiseLite::FractalType_FBm;
    int octaves = 5;
    float lacunarity = 2.0f;
    float gain = 0.5f;
    float landThreshold = 0.5f;
    bool distortCoastline = true;
};
