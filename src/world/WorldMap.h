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
    WorldMap(const std::string& geoJsonPath,
             const std::string& citiesGeoJsonPath,
             const std::string& townsGeoJsonPath,
             const std::string& suburbsGeoJsonPath);
    ~WorldMap() = default;

    WorldMap(const WorldMap&) = delete;
    WorldMap& operator=(const WorldMap&) = delete;

    /* Virtual Methods */
    bool Init() override;

    /* Core Methods */
    void Render(sf::RenderWindow& window, const Camera& camera) const;

    /* Getters */
    float GetWorldWidth() const { return WORLD_WIDTH; }
    float GetWorldHeight() const { return WORLD_HEIGHT; }
    const std::vector<PlaceArea>& GetPlaceAreas() const;
    const MapData& GetMapData() const { return m_mapData; }

private:
    /* File Paths */
    std::string m_geoJsonFilePath;
    std::string m_citiesGeoJsonFilePath;
    std::string m_townsGeoJsonFilePath;
    std::string m_suburbsGeoJsonFilePath;
    
    /* Map Components */
    MapData m_mapData;
    MapLoader m_mapLoader;
    InitializationManager m_initManager;

    /* World Constants */
    static constexpr float WORLD_WIDTH = 3600.0f;
    static constexpr float WORLD_HEIGHT = 1800.0f;
};