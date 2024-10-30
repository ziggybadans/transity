#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "../managers/InitializationManager.h"
#include "../graphics/Camera.h"
#include <nlohmann/json.hpp>
#include "../core/Station.h"
#include "../core/Line.h"

// Forward declaration for Earcut
namespace mapbox {
    template <typename T>
    struct Earcut;
}

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

// Main class representing the world map
class WorldMap : public IInitializable {
public:
    // Constructor and Destructor
    WorldMap(const std::string& geoJsonPath,
        const std::string& citiesGeoJsonPath,
        const std::string& townsGeoJsonPath,
        const std::string& suburbsGeoJsonPath);
    ~WorldMap();

    // Initialize the world map by loading GeoJSON data and creating shapes
    bool Init() override;

    // Render the map on the given window using the specified camera
    void Render(sf::RenderWindow& window, const Camera& camera) const;

    // Get the width and height of the world map
    float GetWorldWidth() const { return WORLD_WIDTH; }
    float GetWorldHeight() const { return WORLD_HEIGHT; }

    // Get the list of place areas on the map
    const std::vector<PlaceArea>& GetPlaceAreas() const;

    // Methods to manage stations and lines
    bool AddStation(const sf::Vector2f& position);  // Add a new station at the specified position
    Station* GetStationAtPosition(const sf::Vector2f& position, float zoomLevel);  // Get a station at a specific position, taking zoom level into account
    void AddLine(const Line& line);  // Add a new line to the world map

    // Get the list of stations and lines
    const std::vector<Station>& GetStations() const;
    const std::vector<Line>& GetLines() const;

    // Methods to manage the line currently being built by the player
    void StartBuildingLine(const sf::Vector2f& startPosition);  // Start building a new line from the specified position
    void AddNodeToCurrentLine(const sf::Vector2f& position);    // Add a new node to the current line being built
    void FinishCurrentLine();                                   // Finish building the current line
    const Line* GetCurrentLine() const;                         // Get a pointer to the current line being built
    bool IsBuildingLine() const;                                // Check if a line is currently being built

    // Set the current mouse position on the map
    void SetCurrentMousePosition(const sf::Vector2f& position);
    sf::Vector2f currentMousePosition;  // Current mouse position on the map

    // Methods to handle curve state for line segments
    void SetNextSegmentCurved(bool curved);     // Set whether the next segment of the line should be curved
    bool GetIsNextSegmentCurved() const;        // Check if the next segment should be curved

    // Methods to manage the selected line
    void SetSelectedLine(Line* line);   // Set the currently selected line
    Line* GetSelectedLine() const;      // Get the currently selected line

    // Return a non-const reference to the list of lines (allows modification)
    std::vector<Line>& GetLines();

    // Method to get a line at a specific position, considering zoom level
    Line* GetLineAtPosition(const sf::Vector2f& position, float zoomLevel);

private:
    // Paths to data files (GeoJSON files for different types of areas)
    std::string geoJsonFilePath;            // File path for general geographic data
    std::string citiesGeoJsonFilePath;      // File path for city data
    std::string townsGeoJsonFilePath;       // File path for town data
    std::string suburbsGeoJsonFilePath;     // File path for suburb data

    // Store the land shapes as VertexArrays (e.g., continents, countries)
    std::vector<sf::VertexArray> landShapes;

    // Store the different place areas (cities, towns, etc.)
    std::vector<PlaceArea> placeAreas;

    // World dimensions (width and height of the map in game units)
    static constexpr float WORLD_WIDTH = 3600.0f;
    static constexpr float WORLD_HEIGHT = 1800.0f;

    // Helper functions for loading GeoJSON data and processing geometry
    bool loadGeoJSON();  // Load GeoJSON data from the specified files

    // Process different types of geometries from the GeoJSON data
    bool processGeometry(const nlohmann::json& geometry, const sf::Color& color, std::vector<sf::VertexArray>& targetShapes, const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);
    bool processPolygon(const nlohmann::json& coordinates, const sf::Color& color, std::vector<sf::VertexArray>& targetShapes, const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);
    bool processMultiPolygon(const nlohmann::json& coordinates, const sf::Color& color, std::vector<sf::VertexArray>& targetShapes, const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);
    bool createVertexArrayFromPolygon(const std::vector<std::vector<sf::Vector2f>>& polygon, const sf::Color& color, std::vector<sf::VertexArray>& targetShapes, const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);

    // Helper to project geographic coordinates (longitude/latitude) to world coordinates
    sf::Vector2f project(const sf::Vector2f& lonLat) const;

    // Constants for colors used in the map
    static const sf::Color LAND_COLOR;     // Color used for land shapes
    static const sf::Color CITY_COLOR;     // Color used for cities
    static const sf::Color TOWN_COLOR;     // Color used for towns
    static const sf::Color SUBURB_COLOR;   // Color used for suburbs

    // Containers to store stations and lines on the map
    std::vector<Station> stations;         // List of all stations
    std::vector<Line> lines;               // List of all lines

    // Variables for managing line-building by the player
    std::unique_ptr<Line> currentLine;     // Pointer to the current line being built
    bool isBuildingLine = false;           // Flag to indicate if a line is being built

    // Variable to handle whether the next segment is curved or straight
    bool isNextSegmentCurved = false;

    // Pointer to the currently selected line
    Line* selectedLine = nullptr;
};
