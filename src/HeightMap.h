// src/HeightMap.h
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class HeightMap {
public:
    HeightMap(const std::string& filepath);

    // Get normalized height (0.0f to 1.0f) at given world coordinates
    float getNormalizedHeight(float worldX, float worldY) const;
    float getScaledHeight(float worldX, float worldY, float scaleX, float scaleY) const;

    // Get dimensions
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    sf::Image image;
    int width;
    int height;

    // Convert world coordinates to image pixels
    void worldToPixel(float worldX, float worldY, int& pixelX, int& pixelY) const;
};
