#include "StationManager.h"
#include <cmath>

StationManager::StationManager() {}

StationManager::~StationManager() {}

bool StationManager::AddStation(const sf::Vector2f& position) {
    stations.emplace_back(position);
    return true;
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
