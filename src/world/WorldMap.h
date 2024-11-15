#pragma once

#include "MapData.h"
#include "MapLoader.h"
#include "../managers/StationManager.h"
#include "../managers/LineManager.h"
#include "../managers/InitializationManager.h"
#include "../graphics/Camera.h"

class WorldMap : public IInitializable {
public:
    // Constructors and Destructor
    WorldMap(const std::string& geoJsonPath,
             const std::string& citiesGeoJsonPath,
             const std::string& townsGeoJsonPath,
             const std::string& suburbsGeoJsonPath);
    ~WorldMap() = default;

    // Delete copy constructor and copy assignment operator to prevent copying
    WorldMap(const WorldMap&) = delete;
    WorldMap& operator=(const WorldMap&) = delete;

    // Initialization
    bool Init() override;

    // Rendering
    void Render(sf::RenderWindow& window, const Camera& camera) const;

    // Accessors
    float GetWorldWidth() const { return WORLD_WIDTH; }
    float GetWorldHeight() const { return WORLD_HEIGHT; }

    const std::vector<PlaceArea>& GetPlaceAreas() const;

    // Station Management
    Station* AddStation(const sf::Vector2f& position);
    Station* GetStationAtPosition(const sf::Vector2f& position, float zoomLevel);
    const std::vector<Station>& GetStations() const;

    // Line Management
    Line* AddLine(std::unique_ptr<Line> line);
    const std::vector<std::unique_ptr<Line>>& GetLines() const;
    Line* GetLineAtPosition(const sf::Vector2f& position, float zoomLevel);

    // Line Building
    void StartBuildingLine(Station* station);
    void StartBuildingBranch(Line* parentLine, const LineNode& startingNode);
    void StartExtendingLine(Line* line, int nodeIndex);
    void AddNodeToCurrentLine(const sf::Vector2f& position);
    void AddStationToCurrentLine(Station* station);
    void FinishCurrentLine();

    const Line* GetCurrentLine() const;
    bool IsBuildingLine() const;
    bool IsExtendingLine() const;
    Line* GetLineBeingExtended() const;
    int GetExtendNodeIndex() const;

    // Selection Management
    void SetSelectedLine(Line* line);
    Line* GetSelectedLine() const;

    void SetSelectedStation(Station* station);
    Station* GetSelectedStation() const;

    // Mouse Input
    void SetCurrentMousePosition(const sf::Vector2f& position);
    sf::Vector2f GetCurrentMousePosition() const;

    // Segment Curve State
    void SetNextSegmentCurved(bool curved);
    bool IsNextSegmentCurved() const;

private:
    // Helper Methods
    Line* GetLineAtPositionRecursive(Line* line, const sf::Vector2f& position, float zoomLevel);

    // Paths to data files (GeoJSON files for different types of areas)
    const std::string geoJsonFilePath;         // File path for general geographic data
    const std::string citiesGeoJsonFilePath;   // File path for city data
    const std::string townsGeoJsonFilePath;    // File path for town data
    const std::string suburbsGeoJsonFilePath;  // File path for suburb data

    // Map Data and Loader
    MapData mapData;
    MapLoader mapLoader;

    // World Dimensions
    static constexpr float WORLD_WIDTH = 3600.0f;
    static constexpr float WORLD_HEIGHT = 1800.0f;

    // Managers
    StationManager stationManager;
    LineManager lineManager;

    // Line Building State
    std::unique_ptr<Line> currentLine;
    bool isBuildingLine = false;
    bool isNextSegmentCurved = false;
    Line* lineBeingExtended = nullptr;  // Non-owning pointer
    int extendNodeIndex = -1;

    // Selection
    Line* selectedLine = nullptr;       // Non-owning pointer
    Station* selectedStation = nullptr; // Non-owning pointer

    // Mouse Position
    sf::Vector2f currentMousePosition;  // Current mouse position on the map
};