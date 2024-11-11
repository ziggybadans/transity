#pragma once

#include "MapData.h"
#include "MapLoader.h"
#include "../managers/StationManager.h"
#include "../managers/LineManager.h"
#include "LineBuilder.h"
#include "../managers/InitializationManager.h"
#include "../graphics/Camera.h"

class WorldMap : public IInitializable {
public:
    WorldMap(const std::string& geoJsonPath,
        const std::string& citiesGeoJsonPath,
        const std::string& townsGeoJsonPath,
        const std::string& suburbsGeoJsonPath);
    ~WorldMap();

    bool Init() override;

    void Render(sf::RenderWindow& window, const Camera& camera) const;

    float GetWorldWidth() const { return WORLD_WIDTH; }
    float GetWorldHeight() const { return WORLD_HEIGHT; }

    const std::vector<PlaceArea>& GetPlaceAreas() const;

    // Methods to manage stations
    bool AddStation(const sf::Vector2f& position);
    Station* GetStationAtPosition(const sf::Vector2f& position, float zoomLevel);
    const std::vector<Station>& GetStations() const;

    // Methods to manage lines
    void AddLine(std::unique_ptr<Line> line);
    const std::vector<std::unique_ptr<Line>>& GetLines() const;
    std::vector<std::unique_ptr<Line>>& GetLines();
    Line* GetLineAtPosition(const sf::Vector2f& position, float zoomLevel);

    // Methods to manage the line currently being built by the player
    void StartBuildingLine(Station* station);
    void AddNodeToCurrentLine(const sf::Vector2f& position);
    void AddStationToCurrentLine(Station* station);
    void FinishCurrentLine();
    const Line* GetCurrentLine() const;
    bool IsBuildingLine() const;

    // Set the current mouse position on the map
    void SetCurrentMousePosition(const sf::Vector2f& position);
    sf::Vector2f currentMousePosition;  // Current mouse position on the map

    // Methods to handle curve state for line segments
    void SetNextSegmentCurved(bool curved);
    bool GetIsNextSegmentCurved() const;

    // Methods to manage the selected line
    void SetSelectedLine(Line* line);
    Line* GetSelectedLine() const;

    void SetSelectedStation(Station* station);
    Station* GetSelectedStation() const;

    // Methods to manage the line currently being built by the player
    void StartBuildingBranch(Line* parentLine, const LineNode& startingNode);

    Line* GetLineAtPositionRecursive(Line* line, const sf::Vector2f& position, float zoomLevel);

    void StartExtendingLine(Line* line, int nodeIndex);

    // Methods to check if we're extending a line and get related information
    bool IsExtendingLine() const;
    Line* GetLineBeingExtended() const;
    int GetExtendNodeIndex() const;

private:
    // Paths to data files (GeoJSON files for different types of areas)
    std::string geoJsonFilePath;            // File path for general geographic data
    std::string citiesGeoJsonFilePath;      // File path for city data
    std::string townsGeoJsonFilePath;       // File path for town data
    std::string suburbsGeoJsonFilePath;     // File path for suburb data

    // Map data and loader
    MapData mapData;
    MapLoader mapLoader;

    // World dimensions (width and height of the map in game units)
    static constexpr float WORLD_WIDTH = 3600.0f;
    static constexpr float WORLD_HEIGHT = 1800.0f;

    // Station and Line managers
    StationManager stationManager;
    LineManager lineManager;

    // Line building
    LineBuilder lineBuilder;

    // Pointer to the currently selected line
    Line* selectedLine = nullptr;

    Station* selectedStation = nullptr; // Initialize to nullptr
};