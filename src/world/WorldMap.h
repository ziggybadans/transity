#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "../managers/InitializationManager.h"
#include "../graphics/Camera.h"
#include <nlohmann/json.hpp>

// Forward declaration for Earcut
namespace mapbox {
    template <typename T>
    struct Earcut;
}

enum class PlaceCategory {
    CapitalCity,
    City,
    Town,
    Suburb,
    Unknown
};

struct City {
    std::string name;
    sf::Vector2f position; // Projected position
    PlaceCategory category;
};

class WorldMap : public IInitializable {
public:
    // Constructor accepts the path to the GeoJSON file and OSM places file
    WorldMap(const std::string& geoJsonPath, const std::string& osmPlacesPath);
    ~WorldMap();

    // Initialize by loading GeoJSON and creating shapes
    bool Init() override;

    // Render the map
    void Render(sf::RenderWindow& window, const Camera& camera) const;

    // Getters for world dimensions
    float GetWorldWidth() const { return WORLD_WIDTH; }
    float GetWorldHeight() const { return WORLD_HEIGHT; }

    // Get cities
    const std::vector<City>& GetCities() const;

private:
    // Paths to data files
    std::string geoJsonFilePath;
    std::string osmPlacesFilePath;

    // Store the shapes as VertexArrays
    std::vector<sf::VertexArray> landShapes;

    // World dimensions (adjust as needed)
    static constexpr float WORLD_WIDTH = 3600.0f;
    static constexpr float WORLD_HEIGHT = 1800.0f;

    // Helper functions for loading GeoJSON data
    bool loadGeoJSON();
    bool processGeometry(const nlohmann::json& geometry);
    bool processPolygon(const nlohmann::json& coordinates, const sf::Color& color);
    bool processMultiPolygon(const nlohmann::json& coordinates, const sf::Color& color);
    bool createVertexArrayFromPolygon(const std::vector<std::vector<sf::Vector2f>>& polygon, const sf::Color& color);

    // Helper functions for loading cities
    bool loadCities();
    bool processFeature(const nlohmann::json& feature);

    // Helper to project geographic coordinates to world coordinates
    sf::Vector2f project(const sf::Vector2f& lonLat) const;

    // Constants
    static const sf::Color LAND_COLOR;
    static const sf::Color MULTI_POLYGON_COLOR;

    std::vector<City> cities;
};
