#pragma once

#include "MapData.h"
#include "MapLoader.h"
#include "../managers/InitializationManager.h"
#include "../graphics/Camera.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>

class WorldMap : public IInitializable {
public:
    // Constructors and Destructor
    WorldMap(const std::string& geoJsonPath,
             const std::string& citiesGeoJsonPath,
             const std::string& townsGeoJsonPath,
             const std::string& suburbsGeoJsonPath);
    ~WorldMap() = default;

    // Delete copy constructor and copy assignment operator to prevent copying
    WorldMap(const WorldMap&) = delete;
    WorldMap& operator=(const WorldMap&) = delete;

    // Initialization
    bool Init() override;

    // Rendering
    void Render(sf::RenderWindow& window, const Camera& camera) const;

    // Accessors
    float GetWorldWidth() const { return WORLD_WIDTH; }
    float GetWorldHeight() const { return WORLD_HEIGHT; }

    const std::vector<PlaceArea>& GetPlaceAreas() const;

private:
    std::string geoJsonFilePath;
    std::string citiesGeoJsonFilePath;
    std::string townsGeoJsonFilePath;
    std::string suburbsGeoJsonFilePath;
    
    MapData mapData;
    MapLoader mapLoader;

    InitializationManager initManager;

    static constexpr float WORLD_WIDTH = 3600.0f;
    static constexpr float WORLD_HEIGHT = 1800.0f;
};