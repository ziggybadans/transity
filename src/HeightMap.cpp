// src/HeightMap.cpp
#include "HeightMap.h"
#include <stdexcept>
#include <algorithm>

HeightMap::HeightMap(const std::string& filepath) {
    if (!image.loadFromFile(filepath)) {
        throw std::runtime_error("Failed to load heightmap image: " + filepath);
    }
    width = image.getSize().x;
    height = image.getSize().y;

    if (width < 2 || height < 2) {
        throw std::runtime_error("Heightmap image must be at least 2x2 pixels.");
    }
}

void HeightMap::worldToPixel(float worldX, float worldY, int& pixelX, int& pixelY) const {
    pixelX = static_cast<int>(worldX);
    pixelY = static_cast<int>(worldY);

    pixelX = std::clamp(pixelX, 0, width - 1);
    pixelY = std::clamp(pixelY, 0, height - 1);
}

float HeightMap::getNormalizedHeight(float worldX, float worldY) const {
    int pixelX, pixelY;
    worldToPixel(worldX, worldY, pixelX, pixelY);
    sf::Color color = image.getPixel(pixelX, pixelY);

    float normalizedHeight = color.r / 255.0f;
    return normalizedHeight;
}

float HeightMap::getScaledHeight(float tileX, float tileY, float invScaleX, float invScaleY) const {
    float scaledX = tileX * invScaleX;
    float scaledY = tileY * invScaleY;

    int pixelX = static_cast<int>(scaledX);
    int pixelY = static_cast<int>(scaledY);

    float fracX = scaledX - pixelX;
    float fracY = scaledY - pixelY;

    pixelX = std::clamp(pixelX, 0, width - 2);
    pixelY = std::clamp(pixelY, 0, height - 2);

    sf::Color c00 = image.getPixel(pixelX, pixelY);
    sf::Color c10 = image.getPixel(pixelX + 1, pixelY);
    sf::Color c01 = image.getPixel(pixelX, pixelY + 1);
    sf::Color c11 = image.getPixel(pixelX + 1, pixelY + 1);

    float h00 = c00.r / 255.0f;
    float h10 = c10.r / 255.0f;
    float h01 = c01.r / 255.0f;
    float h11 = c11.r / 255.0f;

    float h0 = h00 + fracX * (h10 - h00);
    float h1 = h01 + fracX * (h11 - h01);
    float interpolatedHeight = h0 + fracY * (h1 - h0);

    return interpolatedHeight;
}
