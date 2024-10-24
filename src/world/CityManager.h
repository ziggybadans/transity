#pragma once

#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <SFML/Graphics.hpp>
#include "City.h"
#include "WorldMap.h"

class CityManager {
public:
    // Constructor takes the path to the Geonames file and a reference to the WorldMap
    CityManager(const std::string& geonamesFilePath, const WorldMap& worldMap);
    ~CityManager();

    // Initialize and load cities
    bool Init();

    // Get the cities to render based on the current zoom level
    std::vector<City> GetCitiesToRender(float zoomLevel) const;

private:
    std::string geonamesFilePath;
    const WorldMap& worldMap;

    std::vector<City> allCities; // All loaded cities

    mutable std::mutex citiesMutex; // Mutex to protect access to allCities

    // Helper to load cities from Geonames
    bool LoadCities();

    // Convert Geonames coordinates to map coordinates
    sf::Vector2f GeoToMapCoordinates(double latitude, double longitude) const;

    // Determine population threshold based on zoom level
    int DeterminePopulationThreshold(float zoomLevel) const;
};
