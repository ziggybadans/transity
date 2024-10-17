// src/NoiseGenerator.h
#pragma once

#include "FastNoiseLite.h"
#include <vector>
#include <memory>

struct NoiseLayer {
    FastNoiseLite noise;
    float amplitude;
    float frequency;
    FastNoiseLite::NoiseType noiseType;
    int seed;

    // Properties specific to Cellular noise
    FastNoiseLite::CellularDistanceFunction cellularDistanceFunction;
    FastNoiseLite::CellularReturnType cellularReturnType;
    float cellularJitter;

    NoiseLayer(FastNoiseLite::NoiseType type = FastNoiseLite::NoiseType_Perlin,
        float freq = 0.005f, float amp = 1.0f, int s = 1337);

    void configureNoise();
};

class NoiseGenerator {
public:
    NoiseGenerator();

    void addNoiseLayer(const NoiseLayer& layer);
    void removeLastNoiseLayer();
    void initializeDefaultLayers();
    float generateHeight(float x, float y) const;

    float getTotalAmplitude() const { return totalAmplitude; }

    // Getter for noise layers
    const std::vector<NoiseLayer>& getNoiseLayers() const { return noiseLayers; }

    // Setter methods for noise layers
    void setNoiseLayerType(size_t index, FastNoiseLite::NoiseType type);
    void setNoiseLayerFrequency(size_t index, float frequency);
    void setNoiseLayerAmplitude(size_t index, float amplitude);
    void setNoiseLayerSeed(size_t index, int seed);
    void setNoiseLayerCellularDistanceFunction(size_t index, FastNoiseLite::CellularDistanceFunction distanceFunction);
    void setNoiseLayerCellularReturnType(size_t index, FastNoiseLite::CellularReturnType returnType);
    void setNoiseLayerCellularJitter(size_t index, float jitter);

private:
    std::vector<NoiseLayer> noiseLayers;
    float totalAmplitude;

    // Additional helper methods can be added here
};
