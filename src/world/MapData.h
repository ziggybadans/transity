#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

// Enum to categorize places into different types like City, Town, etc.
enum class PlaceCategory {
    CapitalCity,
    City,
    Town,
    Suburb,
    Unknown
};

// Structure to hold information about a specific area/place on the map
struct PlaceArea {
    std::string name;                      // Name of the place
    PlaceCategory category;                // Category of the place (e.g., City, Town)
    sf::VertexArray filledShape;           // The filled shape of the area (e.g., a city boundary)
    sf::VertexArray outline;               // The outline of the area
    sf::FloatRect bounds;                  // Precomputed bounding rectangle for the area (used for optimizations)
};

// Class to hold the map data
class MapData {
public:
    MapData();
    ~MapData();

    // Accessors
    const std::vector<sf::VertexArray>& GetLandShapes() const;
    const std::vector<PlaceArea>& GetPlaceAreas() const;

    // Mutators
    void AddLandShape(const sf::VertexArray& shape);
    void AddPlaceArea(const PlaceArea& area);

    // Constants for colors used in the map
    static const sf::Color LAND_COLOR;
    static const sf::Color CITY_COLOR;
    static const sf::Color TOWN_COLOR;
    static const sf::Color SUBURB_COLOR;

private:
    // Store the land shapes as VertexArrays (e.g., continents, countries)
    std::vector<sf::VertexArray> landShapes;

    // Store the different place areas (cities, towns, etc.)
    std::vector<PlaceArea> placeAreas;
};
