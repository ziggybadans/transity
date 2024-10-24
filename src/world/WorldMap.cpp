#include "WorldMap.h"
#include <iostream>
#include <cmath> // For std::abs

WorldMap::WorldMap(const std::string& highResPath, const std::string& lowResPath, float zoomSwitchLevel)
    : highResImagePath(highResPath),
    lowResImagePath(lowResPath),
    zoomLevelToSwitch(zoomSwitchLevel),
    WORLD_WIDTH(0.0f),
    WORLD_HEIGHT(0.0f)
{}

WorldMap::~WorldMap() {}

bool WorldMap::Init() {
    if (!loadHighRes()) {
        std::cerr << "Failed to load high-resolution map." << std::endl;
        return false;
    }

    if (!loadLowRes()) {
        std::cerr << "Failed to load low-resolution map." << std::endl;
        return false;
    }

    SetWorldDimensions();

    float highResAspect = static_cast<float>(highResTexture.getSize().x) / static_cast<float>(highResTexture.getSize().y);
    float lowResAspect = static_cast<float>(lowResTexture.getSize().x) / static_cast<float>(lowResTexture.getSize().y);

    if (std::abs(highResAspect - lowResAspect) > 0.01f) {
        std::cerr << "High-res and low-res textures have different aspect ratios." << std::endl;
        return false;
    }

    highResSprite.setTexture(highResTexture);
    lowResSprite.setTexture(lowResTexture);

    highResSprite.setOrigin(static_cast<float>(highResTexture.getSize().x) / 2.0f, static_cast<float>(highResTexture.getSize().y) / 2.0f);
    lowResSprite.setOrigin(static_cast<float>(lowResTexture.getSize().x) / 2.0f, static_cast<float>(lowResTexture.getSize().y) / 2.0f);

    highResSprite.setPosition(WORLD_WIDTH / 2.0f, WORLD_HEIGHT / 2.0f);
    lowResSprite.setPosition(WORLD_WIDTH / 2.0f, WORLD_HEIGHT / 2.0f);

    float uniformScaleHigh = WORLD_WIDTH / static_cast<float>(highResTexture.getSize().x);
    float uniformScaleLow = WORLD_WIDTH / static_cast<float>(lowResTexture.getSize().x);

    highResSprite.setScale(uniformScaleHigh, uniformScaleHigh);
    lowResSprite.setScale(uniformScaleLow, uniformScaleLow);

    return true;
}

bool WorldMap::loadHighRes() {
    if (!highResTexture.loadFromFile(highResImagePath)) {
        return false;
    }
    highResTexture.setSmooth(true);
    return true;
}

bool WorldMap::loadLowRes() {
    if (!lowResTexture.loadFromFile(lowResImagePath)) {
        return false;
    }
    lowResTexture.setSmooth(true);
    return true;
}

void WorldMap::SetWorldDimensions() {
    float scaleFactor = 1.0f; // Adjust as needed
    WORLD_WIDTH = highResTexture.getSize().x * scaleFactor;

    float highResAspect = static_cast<float>(highResTexture.getSize().x) / static_cast<float>(highResTexture.getSize().y);
    if (highResAspect <= 0.0f) {
        std::cerr << "Invalid high-res texture aspect ratio." << std::endl;
        WORLD_HEIGHT = 0.0f;
        return;
    }

    WORLD_HEIGHT = WORLD_WIDTH / highResAspect;
}

void WorldMap::Render(sf::RenderWindow& window, const Camera& camera) const {
    float currentZoom = camera.GetZoomLevel();
    const sf::Sprite* spriteToDraw = (currentZoom <= zoomLevelToSwitch) ? &highResSprite : &lowResSprite;

    static std::string lastTexture = "";
    std::string currentTexture = (spriteToDraw == &highResSprite) ? "High-Res" : "Low-Res";

    if (lastTexture != currentTexture) {
        std::cout << "Switched to " << currentTexture << " texture at zoom level: " << currentZoom << std::endl;
        lastTexture = currentTexture;
    }

    window.draw(*spriteToDraw);
}
