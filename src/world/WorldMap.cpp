#include "WorldMap.h"
#include <iostream>

WorldMap::WorldMap(const std::string& geoJsonPath,
    const std::string& citiesGeoJsonPath,
    const std::string& townsGeoJsonPath,
    const std::string& suburbsGeoJsonPath)
    : geoJsonFilePath(geoJsonPath),
    citiesGeoJsonFilePath(citiesGeoJsonPath),
    townsGeoJsonFilePath(townsGeoJsonPath),
    suburbsGeoJsonFilePath(suburbsGeoJsonPath),
    mapLoader(mapData) {}

WorldMap::~WorldMap() {}

bool WorldMap::Init() {
    if (!mapLoader.LoadGeoJSONFiles(geoJsonFilePath, citiesGeoJsonFilePath, townsGeoJsonFilePath, suburbsGeoJsonFilePath)) {
        std::cerr << "Failed to load GeoJSON data." << std::endl;
        return false;
    }
    return true;
}

void WorldMap::Render(sf::RenderWindow& window, const Camera& camera) const {
    // Apply camera view
    sf::View originalView = window.getView();
    window.setView(camera.GetView());

    // Calculate the view rectangle in world coordinates
    sf::Vector2f viewCenter = camera.GetView().getCenter();
    sf::Vector2f viewSize = camera.GetView().getSize();
    sf::FloatRect cameraRect(viewCenter.x - viewSize.x / 2.0f,
        viewCenter.y - viewSize.y / 2.0f,
        viewSize.x,
        viewSize.y);

    // Draw only shapes that intersect with the camera's view
    const auto& landShapes = mapData.GetLandShapes();
    for (const auto& shape : landShapes) {
        sf::FloatRect shapeBounds = shape.getBounds();
        if (cameraRect.intersects(shapeBounds)) {
            window.draw(shape);
        }
    }

    // Restore the original view
    window.setView(originalView);
}

const std::vector<PlaceArea>& WorldMap::GetPlaceAreas() const {
    return mapData.GetPlaceAreas();
}

bool WorldMap::AddStation(const sf::Vector2f& position) {
    return stationManager.AddStation(position);
}

Station* WorldMap::GetStationAtPosition(const sf::Vector2f& position, float zoomLevel) {
    return stationManager.GetStationAtPosition(position, zoomLevel);
}

const std::vector<Station>& WorldMap::GetStations() const {
    return stationManager.GetStations();
}

void WorldMap::AddLine(std::unique_ptr<Line> line) {
    lineManager.AddLine(std::move(line));
}

const std::vector<std::unique_ptr<Line>>& WorldMap::GetLines() const {
    return lineManager.GetLines();
}

std::vector<std::unique_ptr<Line>>& WorldMap::GetLines() {
    return lineManager.GetLines();
}

Line* WorldMap::GetLineAtPosition(const sf::Vector2f& position, float zoomLevel) {
    return lineManager.GetLineAtPosition(position, zoomLevel);
}

void WorldMap::StartBuildingLine(const sf::Vector2f& startPosition) {
    lineBuilder.StartBuildingLine(startPosition);
}

void WorldMap::AddNodeToCurrentLine(const sf::Vector2f& position, bool isStation) {
    lineBuilder.AddNodeToCurrentLine(position, isStation);
}

void WorldMap::FinishCurrentLine() {
    if (lineBuilder.IsBuildingLine()) {
        auto line = lineBuilder.ExtractCurrentLine();
        if (line) {
            line->SetActive(false);
            lineManager.AddLine(std::move(line));
        }
    }
}

const Line* WorldMap::GetCurrentLine() const {
    return lineBuilder.GetCurrentLine();
}

bool WorldMap::IsBuildingLine() const {
    return lineBuilder.IsBuildingLine();
}

void WorldMap::SetCurrentMousePosition(const sf::Vector2f& position) {
    currentMousePosition = position;
}

void WorldMap::SetNextSegmentCurved(bool curved) {
    lineBuilder.SetNextSegmentCurved(curved);
}

bool WorldMap::GetIsNextSegmentCurved() const {
    return lineBuilder.GetIsNextSegmentCurved();
}

void WorldMap::SetSelectedLine(Line* line) {
    selectedLine = line;
}

Line* WorldMap::GetSelectedLine() const {
    return selectedLine;
}