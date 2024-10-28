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

enum class PlaceCategory {
    CapitalCity,
    City,
    Town,
    Suburb,
    Unknown
};

struct PlaceArea {
    std::string name;
    PlaceCategory category;
    sf::VertexArray filledShape;  // The filled shape of the area
    sf::VertexArray outline;      // The outline of the area
    sf::FloatRect bounds;         // Precomputed bounding rectangle
};

class WorldMap : public IInitializable {
public:
    // Constructor and Destructor
    WorldMap(const std::string& geoJsonPath,
        const std::string& citiesGeoJsonPath,
        const std::string& townsGeoJsonPath,
        const std::string& suburbsGeoJsonPath);
    ~WorldMap();

    // Initialize by loading GeoJSON and creating shapes
    bool Init() override;

    // Render the map
    void Render(sf::RenderWindow& window, const Camera& camera) const;

    // Get world dimensions
    float GetWorldWidth() const { return WORLD_WIDTH; }
    float GetWorldHeight() const { return WORLD_HEIGHT; }

    // Get place areas
    const std::vector<PlaceArea>& GetPlaceAreas() const;

    // Methods to manage stations and lines
    bool AddStation(const sf::Vector2f& position);
    Station* GetStationAtPosition(const sf::Vector2f& position, float zoomLevel);
    void AddLine(const Line& line);

    const std::vector<Station>& GetStations() const;
    const std::vector<Line>& GetLines() const;

    // Methods for current line being built
    void StartBuildingLine(const sf::Vector2f& startPosition);
    void AddNodeToCurrentLine(const sf::Vector2f& position); // Updated
    void FinishCurrentLine();
    const Line* GetCurrentLine() const;
    bool IsBuildingLine() const;

    void SetCurrentMousePosition(const sf::Vector2f& position);
    sf::Vector2f currentMousePosition; // Existing member

    // New methods to handle curve state
    void SetNextSegmentCurved(bool curved);
    bool GetIsNextSegmentCurved() const;

    // Methods to manage selected line
    void SetSelectedLine(Line* line);
    Line* GetSelectedLine() const;

    // Method to get line at a position
    Line* GetLineAtPosition(const sf::Vector2f& position, float zoomLevel);

private:
    // Paths to data files
    std::string geoJsonFilePath;
    std::string citiesGeoJsonFilePath;
    std::string townsGeoJsonFilePath;
    std::string suburbsGeoJsonFilePath;

    // Store the shapes as VertexArrays
    std::vector<sf::VertexArray> landShapes;

    // Store place areas
    std::vector<PlaceArea> placeAreas;

    // World dimensions (adjust as needed)
    static constexpr float WORLD_WIDTH = 3600.0f;
    static constexpr float WORLD_HEIGHT = 1800.0f;

    // Helper functions for loading GeoJSON data
    bool loadGeoJSON();
    bool processGeometry(const nlohmann::json& geometry, const sf::Color& color, std::vector<sf::VertexArray>& targetShapes, const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);
    bool processPolygon(const nlohmann::json& coordinates, const sf::Color& color, std::vector<sf::VertexArray>& targetShapes, const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);
    bool processMultiPolygon(const nlohmann::json& coordinates, const sf::Color& color, std::vector<sf::VertexArray>& targetShapes, const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);
    bool createVertexArrayFromPolygon(const std::vector<std::vector<sf::Vector2f>>& polygon, const sf::Color& color, std::vector<sf::VertexArray>& targetShapes, const std::string& name = "", PlaceCategory category = PlaceCategory::Unknown);

    // Helper to project geographic coordinates to world coordinates
    sf::Vector2f project(const sf::Vector2f& lonLat) const;

    // Constants
    static const sf::Color LAND_COLOR;

    // Colors for different place categories
    static const sf::Color CITY_COLOR;
    static const sf::Color TOWN_COLOR;
    static const sf::Color SUBURB_COLOR;

    // New members for stations and lines
    std::vector<Station> stations;
    std::vector<Line> lines;

    // For line building
    std::unique_ptr<Line> currentLine;
    bool isBuildingLine = false;

    // New members for curve handling
    bool isNextSegmentCurved = false;

    // For line selection
    Line* selectedLine = nullptr;
};
