#pragma once

#include "MapData.h"
#include <string>
#include <nlohmann/json.hpp>
#include <mapbox/earcut.hpp>

class MapLoader {
public:
    MapLoader(MapData& mapData);
    ~MapLoader();

    bool LoadGeoJSONFiles(
        const std::string& landGeoJsonPath,
        const std::string& citiesGeoJsonPath,
        const std::string& townsGeoJsonPath,
        const std::string& suburbsGeoJsonPath
    );

private:
    // Helper functions for loading GeoJSON data and processing geometry
    bool loadGeoJSON(const std::string& geoJsonFilePath, const sf::Color& color, PlaceCategory category = PlaceCategory::Unknown);
    bool processGeometry(const nlohmann::json& geometry, const sf::Color& color, const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);
    bool processPolygon(const nlohmann::json& coordinates, const sf::Color& color, const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);
    bool processMultiPolygon(const nlohmann::json& coordinates, const sf::Color& color, const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);
    bool createVertexArrayFromPolygon(const std::vector<std::vector<sf::Vector2f>>& polygon, const sf::Color& color, const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);

    // Helper to project geographic coordinates (longitude/latitude) to world coordinates
    sf::Vector2f project(const sf::Vector2f& lonLat) const;

    MapData& mapData;

    // World dimensions (width and height of the map in game units)
    static constexpr float WORLD_WIDTH = 3600.0f;
    static constexpr float WORLD_HEIGHT = 1800.0f;
};
