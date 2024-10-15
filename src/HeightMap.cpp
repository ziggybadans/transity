// src/HeightMap.cpp
#include "HeightMap.h"
#include <stdexcept>

HeightMap::HeightMap(const std::string& filepath) {
    if (!image.loadFromFile(filepath)) {
        throw std::runtime_error("Failed to load heightmap image: " + filepath);
    }
    width = image.getSize().x;
    height = image.getSize().y;
}

void HeightMap::worldToPixel(float worldX, float worldY, int& pixelX, int& pixelY) const {
    // Assuming one pixel per tile and (0,0) at top-left
    pixelX = static_cast<int>(worldX);
    pixelY = static_cast<int>(worldY);

    // Clamp to image boundaries
    if (pixelX < 0) pixelX = 0;
    if (pixelY < 0) pixelY = 0;
    if (pixelX >= width) pixelX = width - 1;
    if (pixelY >= height) pixelY = height - 1;
}

float HeightMap::getHeight(float worldX, float worldY) const {
    int pixelX, pixelY;
    worldToPixel(worldX, worldY, pixelX, pixelY);
    sf::Color color = image.getPixel(pixelX, pixelY);

    // Assuming grayscale, so R = G = B
    float normalizedHeight = color.r / 255.0f; // 0.0f to 1.0f
    return normalizedHeight;
}

float HeightMap::getScaledHeight(float tileX, float tileY, float invScaleX, float invScaleY) const {
    // Convert tile coordinates to heightmap pixel coordinates
    int pixelX = static_cast<int>(tileX * invScaleX);
    int pixelY = static_cast<int>(tileY * invScaleY);

    // Clamp to image boundaries
    pixelX = std::clamp(pixelX, 0, width - 1);
    pixelY = std::clamp(pixelY, 0, height - 1);

    sf::Color color = image.getPixel(pixelX, pixelY);
    float normalizedHeight = color.r / 255.0f; // Assuming grayscale, R = G = B
    return normalizedHeight;
}
