#include "StationManager.h"
#include <cmath>

StationManager::StationManager() : selectedStation(nullptr) {}

StationManager::~StationManager() {}

Station* StationManager::AddStation(const sf::Vector2f& position) {
    stations.emplace_back(position);
    return &stations.back(); // Return pointer to the newly added station
}

Station* StationManager::GetStationAtPosition(const sf::Vector2f& position, float zoomLevel) {
    for (auto& station : stations) {
        sf::Vector2f stationPos = station.GetPosition();
        float baseRadius = 10.0f;
        float scaledRadius = baseRadius * zoomLevel;

        if (std::hypot(position.x - stationPos.x, position.y - stationPos.y) <= scaledRadius) {
            return &station;
        }
    }
    return nullptr;
}

const std::vector<Station>& StationManager::GetStations() const {
    return stations;
}

Station* StationManager::GetSelectedStation() const {
    return selectedStation;
}

void StationManager::SetSelectedStation(Station* station) {
    selectedStation = station;
}