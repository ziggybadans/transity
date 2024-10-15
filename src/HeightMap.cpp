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

float HeightMap::getNormalizedHeight(float worldX, float worldY) const {
    int pixelX, pixelY;
    worldToPixel(worldX, worldY, pixelX, pixelY);
    sf::Color color = image.getPixel(pixelX, pixelY);

    // Assuming grayscale, so R = G = B
    float normalizedHeight = color.r / 255.0f; // 0.0f to 1.0f
    return normalizedHeight;
}

float HeightMap::getScaledHeight(float tileX, float tileY, float invScaleX, float invScaleY) const {
    // Convert tile coordinates to heightmap pixel coordinates
    float scaledX = tileX * invScaleX;
    float scaledY = tileY * invScaleY;

    // Use interpolation for smoother height transitions
    int pixelX = static_cast<int>(scaledX);
    int pixelY = static_cast<int>(scaledY);

    float fracX = scaledX - pixelX;
    float fracY = scaledY - pixelY;

    // Clamp to ensure we don't go out of bounds
    pixelX = std::clamp(pixelX, 0, width - 2);
    pixelY = std::clamp(pixelY, 0, height - 2);

    // Bilinear interpolation for smoother height values
    sf::Color c00 = image.getPixel(pixelX, pixelY);
    sf::Color c10 = image.getPixel(pixelX + 1, pixelY);
    sf::Color c01 = image.getPixel(pixelX, pixelY + 1);
    sf::Color c11 = image.getPixel(pixelX + 1, pixelY + 1);

    float h00 = c00.r / 255.0f;
    float h10 = c10.r / 255.0f;
    float h01 = c01.r / 255.0f;
    float h11 = c11.r / 255.0f;

    // Interpolate
    float h0 = h00 + fracX * (h10 - h00);
    float h1 = h01 + fracX * (h11 - h01);
    float interpolatedHeight = h0 + fracY * (h1 - h0);

    return interpolatedHeight;
}
