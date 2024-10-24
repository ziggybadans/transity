#include "CityManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

CityManager::CityManager(const std::string& geonamesFilePath_, const WorldMap& wm)
    : geonamesFilePath(geonamesFilePath_), worldMap(wm) {}

CityManager::~CityManager() {}

bool CityManager::Init() {
    return LoadCities();
}

bool CityManager::LoadCities() {
    std::ifstream infile(geonamesFilePath);
    if (!infile.is_open()) {
        std::cerr << "Failed to open Geonames file: " << geonamesFilePath << std::endl;
        return false;
    }

    std::string line;
    int loadedCount = 0;
    while (std::getline(infile, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, '\t')) {
            tokens.push_back(token);
        }

        if (tokens.size() < 15) {
            continue; // Incomplete line
        }

        std::string name = tokens[1];
        double latitude = 0.0;
        double longitude = 0.0;
        int population = 0;

        try {
            latitude = std::stod(tokens[4]);
            longitude = std::stod(tokens[5]);
            population = std::stoi(tokens[14]);
        }
        catch (const std::invalid_argument&) {
            continue; // Invalid data
        }

        if (population < 1000) {
            continue; // Skip small towns
        }

        sf::Vector2f mapPos = GeoToMapCoordinates(latitude, longitude);

        {
            std::lock_guard<std::mutex> lock(citiesMutex);
            allCities.emplace_back(name, mapPos, population);
            ++loadedCount;
        }
    }

    std::cout << "CityManager: Loaded " << loadedCount << " cities." << std::endl;

    return true;
}

sf::Vector2f CityManager::GeoToMapCoordinates(double latitude, double longitude) const {
    // Simple equirectangular projection
    float mapWidth = worldMap.GetWorldWidth();
    float mapHeight = worldMap.GetWorldHeight();

    float x = static_cast<float>((longitude + 180.0) / 360.0 * mapWidth);
    float y = static_cast<float>((90.0 - latitude) / 180.0 * mapHeight);

    return sf::Vector2f(x, y);
}

int CityManager::DeterminePopulationThreshold(float zoomLevel) const {
    // Prevent division by zero or negative zoom levels
    if (zoomLevel <= 0.0f) zoomLevel = 0.01f;

    // Adjust the base threshold according to the zoom level
    // As zoomLevel increases (zooming out), threshold increases
    int threshold = static_cast<int>(100000.0f * zoomLevel);

    // Clamp the threshold to a reasonable maximum
    threshold = std::min(threshold, 1000000);

    return threshold;
}

std::vector<City> CityManager::GetCitiesToRender(float zoomLevel) const {
    int threshold = DeterminePopulationThreshold(zoomLevel);

    std::vector<City> citiesToRender;
    {
        std::lock_guard<std::mutex> lock(citiesMutex);
        // Reserve to prevent reallocations
        citiesToRender.reserve(allCities.size());

        for (const auto& city : allCities) {
            if (city.population >= threshold) {
                citiesToRender.emplace_back(city);
            }
        }
    }

    return citiesToRender;
}
