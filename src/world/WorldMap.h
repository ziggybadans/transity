#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <mutex>
#include <string>
#include "../managers/InitializationManager.h"
#include "../graphics/Camera.h"

class WorldMap : public IInitializable {
public:
    WorldMap(const std::string& highResPath, const std::string& lowResPath, float zoomSwitchLevel);
    ~WorldMap();

    // Initialize by loading textures
    bool Init() override;

    // Render the appropriate map based on camera zoom
    void Render(sf::RenderWindow& window, const class Camera& camera) const;

    // Getters for world dimensions
    float GetWorldWidth() const { return WORLD_WIDTH; }
    float GetWorldHeight() const { return WORLD_HEIGHT; }

private:
    std::string highResImagePath;
    std::string lowResImagePath;

    sf::Texture highResTexture;
    sf::Texture lowResTexture;

    sf::Sprite highResSprite;
    sf::Sprite lowResSprite;

    bool loadHighRes();
    bool loadLowRes();

    // World dimensions
    float WORLD_WIDTH;
    float WORLD_HEIGHT;

    // Zoom threshold to switch textures
    float zoomLevelToSwitch;

    // Helper to set world dimensions based on texture size
    void SetWorldDimensions();
};
