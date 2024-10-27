#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "../managers/InitializationManager.h"
#include "../graphics/Camera.h"

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
    void Render(sf::RenderWindow& window, const class Camera& camera) const;

    // Getters for world dimensions
    float GetWorldWidth() const { return WORLD_WIDTH; }
    float GetWorldHeight() const { return WORLD_HEIGHT; }

    // Load city data from OSM places JSON
    bool loadCities(const std::string& osmPlacesFilePath);

    // Get cities
    const std::vector<City>& GetCities() const;

private:
    std::string geoJsonFilePath;
    std::string osmPlacesFilePath;

    // Store the shapes as VertexArrays
    std::vector<sf::VertexArray> landShapes;

    // World dimensions (adjust as needed)
    const float WORLD_WIDTH = 3600.0f;
    const float WORLD_HEIGHT = 1800.0f;

    // Helper to load GeoJSON
    bool loadGeoJSON();

    // Helper to project geographic coordinates to world coordinates
    sf::Vector2f project(const sf::Vector2f& lonLat) const;

    std::vector<City> cities;
};
