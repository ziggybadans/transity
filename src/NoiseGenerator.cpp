// src/NoiseGenerator.cpp
#include "NoiseGenerator.h"

NoiseLayer::NoiseLayer(FastNoiseLite::NoiseType type, float freq, float amp, int s)
    : amplitude(amp), frequency(freq), noiseType(type), seed(s),
    cellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean),
    cellularReturnType(FastNoiseLite::CellularReturnType_Distance2),
    cellularJitter(0.5f) // Default jitter
{
    noise.SetNoiseType(noiseType);
    noise.SetFrequency(frequency);
    noise.SetSeed(seed);
    configureNoise();
}

void NoiseLayer::configureNoise() {
    if (noiseType == FastNoiseLite::NoiseType_Cellular) {
        noise.SetCellularDistanceFunction(cellularDistanceFunction);
        noise.SetCellularReturnType(cellularReturnType);
        noise.SetCellularJitter(cellularJitter);
    }
    // Add configurations for other noise types if necessary
}

NoiseGenerator::NoiseGenerator()
    : totalAmplitude(0.0f) {
    initializeDefaultLayers();
}

void NoiseGenerator::addNoiseLayer(const NoiseLayer& layer) {
    noiseLayers.emplace_back(layer);
    totalAmplitude += layer.amplitude;
}

void NoiseGenerator::removeLastNoiseLayer() {
    if (!noiseLayers.empty()) {
        totalAmplitude -= noiseLayers.back().amplitude;
        noiseLayers.pop_back();
    }
}

void NoiseGenerator::initializeDefaultLayers() {
    // Add initial noise layers
    NoiseLayer perlinLayer(FastNoiseLite::NoiseType_Perlin, 0.0075f, 0.6f, 1337);
    addNoiseLayer(perlinLayer);

    NoiseLayer cellularLayer(FastNoiseLite::NoiseType_Cellular, 0.001f, 1.0f, 42);
    cellularLayer.cellularDistanceFunction = FastNoiseLite::CellularDistanceFunction_EuclideanSq;
    cellularLayer.cellularReturnType = FastNoiseLite::CellularReturnType_Distance2;
    cellularLayer.cellularJitter = 0.8f;
    cellularLayer.configureNoise();
    addNoiseLayer(cellularLayer);
}

float NoiseGenerator::generateHeight(float x, float y) const {
    float height = 0.0f;
    for (const auto& layer : noiseLayers) {
        float noiseValue = layer.noise.GetNoise(x, y, 0.0f);
        noiseValue = (noiseValue + 1.0f) / 2.0f;
        height += noiseValue * layer.amplitude;
    }
    return totalAmplitude != 0.0f ? height / totalAmplitude : 0.0f;
}

// Setter implementations
void NoiseGenerator::setNoiseLayerType(size_t index, FastNoiseLite::NoiseType type) {
    if (index < noiseLayers.size()) {
        noiseLayers[index].noiseType = type;
        noiseLayers[index].noise.SetNoiseType(type);
        noiseLayers[index].configureNoise();
    }
}

void NoiseGenerator::setNoiseLayerFrequency(size_t index, float frequency) {
    if (index < noiseLayers.size()) {
        noiseLayers[index].frequency = frequency;
        noiseLayers[index].noise.SetFrequency(frequency);
    }
}

void NoiseGenerator::setNoiseLayerAmplitude(size_t index, float amplitude) {
    if (index < noiseLayers.size()) {
        totalAmplitude -= noiseLayers[index].amplitude;
        noiseLayers[index].amplitude = amplitude;
        totalAmplitude += noiseLayers[index].amplitude;
    }
}

void NoiseGenerator::setNoiseLayerSeed(size_t index, int seed) {
    if (index < noiseLayers.size()) {
        noiseLayers[index].seed = seed;
        noiseLayers[index].noise.SetSeed(seed);
    }
}

void NoiseGenerator::setNoiseLayerCellularDistanceFunction(size_t index, FastNoiseLite::CellularDistanceFunction distanceFunction) {
    if (index < noiseLayers.size() && noiseLayers[index].noiseType == FastNoiseLite::NoiseType_Cellular) {
        noiseLayers[index].cellularDistanceFunction = distanceFunction;
        noiseLayers[index].noise.SetCellularDistanceFunction(distanceFunction);
    }
}

void NoiseGenerator::setNoiseLayerCellularReturnType(size_t index, FastNoiseLite::CellularReturnType returnType) {
    if (index < noiseLayers.size() && noiseLayers[index].noiseType == FastNoiseLite::NoiseType_Cellular) {
        noiseLayers[index].cellularReturnType = returnType;
        noiseLayers[index].noise.SetCellularReturnType(returnType);
    }
}

void NoiseGenerator::setNoiseLayerCellularJitter(size_t index, float jitter) {
    if (index < noiseLayers.size() && noiseLayers[index].noiseType == FastNoiseLite::NoiseType_Cellular) {
        noiseLayers[index].cellularJitter = jitter;
        noiseLayers[index].noise.SetCellularJitter(jitter);
    }
}
