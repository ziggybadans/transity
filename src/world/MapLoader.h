#pragma once

#include "MapData.h"
#include <string>
#include <nlohmann/json.hpp>
#include <mapbox/earcut.hpp>

class MapLoader {
public:
    MapLoader(MapData& mapData);
    ~MapLoader();

    /* Core Loading Methods */
    bool LoadGeoJSONFiles(const std::string& landGeoJsonPath,
                         const std::string& citiesGeoJsonPath,
                         const std::string& townsGeoJsonPath,
                         const std::string& suburbsGeoJsonPath);

private:
    /* GeoJSON Processing Methods */
    bool LoadGeoJSON(const std::string& geoJsonFilePath, const sf::Color& color, 
                    PlaceCategory category = PlaceCategory::Unknown);
    bool ProcessGeometry(const nlohmann::json& geometry, const sf::Color& color,
                        const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);
    bool ProcessPolygon(const nlohmann::json& coordinates, const sf::Color& color,
                       const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);
    bool ProcessMultiPolygon(const nlohmann::json& coordinates, const sf::Color& color,
                            const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);
    bool CreateVertexArrayFromPolygon(const std::vector<std::vector<sf::Vector2f>>& polygon,
                                     const sf::Color& color, const std::string& name = "",
                                     PlaceCategory category = PlaceCategory::Unknown);
    sf::Vector2f Project(const sf::Vector2f& lonLat) const;

    /* Map Data */
    MapData& m_mapData;

    /* World Constants */
    static constexpr float WORLD_WIDTH = 3600.0f;
    static constexpr float WORLD_HEIGHT = 1800.0f;
};
