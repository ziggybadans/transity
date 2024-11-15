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
      mapLoader(mapData)
{
}

bool WorldMap::Init() {
    if (!mapLoader.LoadGeoJSONFiles(geoJsonFilePath,
                                    citiesGeoJsonFilePath,
                                    townsGeoJsonFilePath,
                                    suburbsGeoJsonFilePath)) {
        std::cerr << "Failed to load GeoJSON data." << std::endl;
        return false;
    }
    return true;
}

void WorldMap::Render(sf::RenderWindow& window, const Camera& camera) const {
    sf::View originalView = window.getView();
    window.setView(camera.GetView());

    // Calculate the view rectangle in world coordinates
    sf::Vector2f viewCenter = camera.GetView().getCenter();
    sf::Vector2f viewSize = camera.GetView().getSize();
    sf::FloatRect cameraRect(
        viewCenter.x - viewSize.x / 2.0f,
        viewCenter.y - viewSize.y / 2.0f,
        viewSize.x,
        viewSize.y
    );

    // Draw only shapes that intersect with the camera's view
    const auto& landShapes = mapData.GetLandShapes();
    for (const auto& shape : landShapes) {
        sf::FloatRect shapeBounds = shape.getBounds();
        if (cameraRect.intersects(shapeBounds)) {
            window.draw(shape);
        }
    }

    window.setView(originalView);
}

const std::vector<PlaceArea>& WorldMap::GetPlaceAreas() const {
    return mapData.GetPlaceAreas();
}

Station* WorldMap::AddStation(const sf::Vector2f& position) {
    return stationManager.AddStation(position);
}

Station* WorldMap::GetStationAtPosition(const sf::Vector2f& position, float zoomLevel) {
    return stationManager.GetStationAtPosition(position, zoomLevel);
}

const std::vector<Station>& WorldMap::GetStations() const {
    return stationManager.GetStations();
}

Line* WorldMap::AddLine(std::unique_ptr<Line> line) {
    return lineManager.AddLine(std::move(line));
}

const std::vector<std::unique_ptr<Line>>& WorldMap::GetLines() const {
    return lineManager.GetLines();
}

Line* WorldMap::GetLineAtPosition(const sf::Vector2f& position, float zoomLevel) {
    for (const auto& line : lineManager.GetLines()) {
        Line* foundLine = GetLineAtPositionRecursive(line.get(), position, zoomLevel);
        if (foundLine) {
            return foundLine;
        }
    }
    return nullptr;
}

Line* WorldMap::GetLineAtPositionRecursive(Line* line, const sf::Vector2f& position, float zoomLevel) {
    if (line->IsPointOnLine(position, zoomLevel)) {
        return line;
    }
    // Check child lines recursively
    for (const auto& childLine : line->GetChildLines()) {
        Line* foundLine = GetLineAtPositionRecursive(childLine.get(), position, zoomLevel);
        if (foundLine) {
            return foundLine;
        }
    }
    return nullptr;
}

void WorldMap::StartBuildingLine(Station* station) {
    currentLine = std::make_unique<Line>();
    currentLine->AddNode(station);
    isBuildingLine = true;
    lineBeingExtended = nullptr;
    extendNodeIndex = -1;
}

void WorldMap::StartBuildingBranch(Line* parentLine, const LineNode& startingNode) {
    currentLine = std::make_unique<Line>(parentLine);
    currentLine->SetColor(parentLine->GetColor());
    currentLine->SetThickness(parentLine->GetThickness());
    currentLine->SetSpeed(parentLine->GetSpeed());

    if (startingNode.IsStation()) {
        currentLine->AddNode(startingNode.station);
    } else {
        currentLine->AddNode(startingNode.position);
    }

    isBuildingLine = true;
    lineBeingExtended = nullptr;
    extendNodeIndex = -1;
}

void WorldMap::StartExtendingLine(Line* line, int nodeIndex) {
    if (!line) {
        std::cerr << "WorldMap::StartExtendingLine called with null line." << std::endl;
        return;
    }
    if (nodeIndex != 0 && nodeIndex != static_cast<int>(line->GetNodes().size()) - 1) {
        std::cerr << "WorldMap::StartExtendingLine called with invalid node index." << std::endl;
        return;
    }

    currentLine.reset();
    isBuildingLine = true;
    lineBeingExtended = line;
    extendNodeIndex = nodeIndex;
}

void WorldMap::AddNodeToCurrentLine(const sf::Vector2f& position) {
    if (isBuildingLine) {
        if (lineBeingExtended) {
            lineBeingExtended->AddNode(position);
        } else if (currentLine) {
            currentLine->AddNode(position);
        }
    }
}

void WorldMap::AddStationToCurrentLine(Station* station) {
    if (isBuildingLine) {
        if (lineBeingExtended) {
            lineBeingExtended->AddNode(station);
        } else if (currentLine) {
            currentLine->AddNode(station);
        }
    }
}

void WorldMap::FinishCurrentLine() {
    if (isBuildingLine) {
        if (lineBeingExtended) {
            lineBeingExtended->SetActive(false);
            lineBeingExtended = nullptr;
            extendNodeIndex = -1;
        } else if (currentLine) {
            currentLine->SetActive(false);
            AddLine(std::move(currentLine));
        }
        isBuildingLine = false;
    }
}

const Line* WorldMap::GetCurrentLine() const {
    return currentLine.get();
}

bool WorldMap::IsBuildingLine() const {
    return isBuildingLine;
}

bool WorldMap::IsExtendingLine() const {
    return lineBeingExtended != nullptr;
}

Line* WorldMap::GetLineBeingExtended() const {
    return lineBeingExtended;
}

int WorldMap::GetExtendNodeIndex() const {
    return extendNodeIndex;
}

void WorldMap::SetSelectedLine(Line* line) {
    selectedLine = line;
}

Line* WorldMap::GetSelectedLine() const {
    return selectedLine;
}

void WorldMap::SetSelectedStation(Station* station) {
    selectedStation = station;
}

Station* WorldMap::GetSelectedStation() const {
    return selectedStation;
}

void WorldMap::SetCurrentMousePosition(const sf::Vector2f& position) {
    currentMousePosition = position;
}

sf::Vector2f WorldMap::GetCurrentMousePosition() const {
    return currentMousePosition;
}

void WorldMap::SetNextSegmentCurved(bool curved) {
    isNextSegmentCurved = curved;
}

bool WorldMap::IsNextSegmentCurved() const {
    return isNextSegmentCurved;
}