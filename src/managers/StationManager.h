#pragma once

#include "../core/Station.h"
#include <SFML/System.hpp>
#include <vector>

class StationManager {
public:
    StationManager();
    ~StationManager();

    bool AddStation(const sf::Vector2f& position);
    Station* GetStationAtPosition(const sf::Vector2f& position, float zoomLevel);
    const std::vector<Station>& GetStations() const;

private:
    // Containers to store stations
    std::vector<Station> stations;
};
