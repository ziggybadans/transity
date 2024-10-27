#include "WorldMap.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <mapbox/earcut.hpp>

using json = nlohmann::json;

// Mapbox Earcut namespace specialization for SFML's sf::Vector2f
namespace mapbox {
    namespace util {
        template <>
        struct nth<0, sf::Vector2f> {
            static auto get(const sf::Vector2f& t) {
                return t.x;
            }
        };
        template <>
        struct nth<1, sf::Vector2f> {
            static auto get(const sf::Vector2f& t) {
                return t.y;
            }
        };
    }
}

// Define static constants
const sf::Color WorldMap::LAND_COLOR = sf::Color(34, 139, 34);       // ForestGreen
const sf::Color WorldMap::MULTI_POLYGON_COLOR = sf::Color(231, 232, 234); // Light Gray

WorldMap::WorldMap(const std::string& geoJsonPath, const std::string& osmPlacesPath)
    : geoJsonFilePath(geoJsonPath),
    osmPlacesFilePath(osmPlacesPath) {}

WorldMap::~WorldMap() {}

bool WorldMap::Init() {
    if (!loadGeoJSON()) {
        std::cerr << "Failed to load GeoJSON data." << std::endl;
        return false;
    }

    if (!loadCities()) {
        std::cerr << "Failed to load city data from OSM places." << std::endl;
        return false;
    }

    return true;
}

bool WorldMap::loadGeoJSON() {
    std::ifstream inFile(geoJsonFilePath);
    if (!inFile.is_open()) {
        std::cerr << "Could not open GeoJSON file: " << geoJsonFilePath << std::endl;
        return false;
    }

    json geoData;
    try {
        inFile >> geoData;
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON Parsing error: " << e.what() << std::endl;
        return false;
    }

    // Validate GeoJSON structure
    if (!geoData.contains("geometries") || !geoData["geometries"].is_array()) {
        std::cerr << "Invalid GeoJSON structure: Missing 'geometries' array." << std::endl;
        return false;
    }

    const auto& geometries = geoData["geometries"];

    for (const auto& geometry : geometries) {
        if (!processGeometry(geometry)) {
            std::cerr << "Failed to process geometry." << std::endl;
        }
    }

    std::cout << "Loaded " << landShapes.size() << " land shapes from GeoJSON." << std::endl;
    return true;
}

bool WorldMap::processGeometry(const json& geometry) {
    if (!geometry.contains("type") || !geometry.contains("coordinates")) {
        std::cerr << "Invalid geometry object: Missing 'type' or 'coordinates'." << std::endl;
        return false;
    }

    std::string geometryType = geometry["type"];
    const auto& coordinates = geometry["coordinates"];

    if (geometryType == "Polygon") {
        return processPolygon(coordinates, LAND_COLOR);
    }
    else if (geometryType == "MultiPolygon") {
        return processMultiPolygon(coordinates, MULTI_POLYGON_COLOR);
    }
    else {
        std::cerr << "Unsupported geometry type: " << geometryType << std::endl;
        return false;
    }
}

bool WorldMap::processPolygon(const json& coordinates, const sf::Color& color) {
    if (!coordinates.is_array()) {
        std::cerr << "Invalid Polygon coordinates." << std::endl;
        return false;
    }

    std::vector<std::vector<sf::Vector2f>> polygon;
    for (const auto& ring : coordinates) {
        if (!ring.is_array()) {
            std::cerr << "Invalid ring in Polygon." << std::endl;
            continue;
        }

        std::vector<sf::Vector2f> ringPoints;
        for (const auto& point : ring) {
            if (!point.is_array() || point.size() < 2) {
                std::cerr << "Invalid point in Polygon ring." << std::endl;
                continue;
            }

            float lon = point[0];
            float lat = point[1];
            ringPoints.push_back(project({ lon, lat }));
        }

        if (!ringPoints.empty()) {
            polygon.push_back(ringPoints);
        }
    }

    return !polygon.empty() && createVertexArrayFromPolygon(polygon, color);
}

bool WorldMap::processMultiPolygon(const json& coordinates, const sf::Color& color) {
    if (!coordinates.is_array()) {
        std::cerr << "Invalid MultiPolygon coordinates." << std::endl;
        return false;
    }

    for (const auto& polygons : coordinates) {
        if (!processPolygon(polygons, color)) {
            std::cerr << "Failed to process Polygon in MultiPolygon." << std::endl;
        }
    }

    return true;
}

bool WorldMap::createVertexArrayFromPolygon(const std::vector<std::vector<sf::Vector2f>>& polygon, const sf::Color& color) {
    // Triangulate the polygon using Earcut
    std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);

    // Flatten the polygon points
    std::vector<sf::Vector2f> flattenedPoints;
    for (const auto& ring : polygon) {
        flattenedPoints.insert(flattenedPoints.end(), ring.begin(), ring.end());
    }

    // Create a VertexArray
    sf::VertexArray va(sf::Triangles, indices.size());
    for (size_t i = 0; i < indices.size(); ++i) {
        if (indices[i] < flattenedPoints.size()) {
            va[i].position = flattenedPoints[indices[i]];
            va[i].color = color;
        }
        else {
            std::cerr << "Index out of bounds during triangulation." << std::endl;
            return false;
        }
    }

    landShapes.push_back(va);
    return true;
}

sf::Vector2f WorldMap::project(const sf::Vector2f& lonLat) const {
    // Simple equirectangular projection
    float x = (lonLat.x + 180.0f) / 360.0f * WORLD_WIDTH;
    float y = (90.0f - lonLat.y) / 180.0f * WORLD_HEIGHT;
    return { x, y };
}

void WorldMap::Render(sf::RenderWindow& window, const Camera& camera) const {
    // Apply camera view
    sf::View originalView = window.getView();
    window.setView(camera.GetView());

    // Draw all land shapes
    for (const auto& shape : landShapes) {
        window.draw(shape);
    }

    // Restore the original view
    window.setView(originalView);
}

const std::vector<City>& WorldMap::GetCities() const {
    return cities;
}

bool WorldMap::loadCities() {
    std::ifstream inFile(osmPlacesFilePath);
    if (!inFile.is_open()) {
        std::cerr << "Could not open OSM Places JSON file: " << osmPlacesFilePath << std::endl;
        return false;
    }

    json osmData;
    try {
        inFile >> osmData;
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON Parsing error in OSM Places: " << e.what() << std::endl;
        return false;
    }

    // Validate OSM Places structure
    if (!osmData.contains("features") || !osmData["features"].is_array()) {
        std::cerr << "Invalid OSM Places JSON structure: Missing 'features' array." << std::endl;
        return false;
    }

    const auto& features = osmData["features"];

    for (const auto& feature : features) {
        processFeature(feature);
    }

    std::cout << "Loaded " << cities.size() << " cities from OSM Places." << std::endl;
    return true;
}

bool WorldMap::processFeature(const json& feature) {
    if (!feature.contains("properties") || !feature["properties"].is_object()) {
        return false;
    }

    const auto& properties = feature["properties"];
    if (!properties.contains("name")) {
        return false;
    }

    std::string name = properties["name"];
    PlaceCategory category = PlaceCategory::Unknown;

    if (properties.contains("capital")) {
        category = PlaceCategory::CapitalCity;
    }
    else if (properties.contains("place")) {
        std::string place = properties["place"];
        if (place == "city") {
            category = PlaceCategory::City;
        }
        else if (place == "town") {
            category = PlaceCategory::Town;
        }
        else if (place == "suburb") {
            category = PlaceCategory::Suburb;
        }
    }

    if (category == PlaceCategory::Unknown) {
        return false;
    }

    if (!feature.contains("geometry") || !feature["geometry"].is_object()) {
        return false;
    }

    const auto& geometry = feature["geometry"];
    if (geometry["type"] != "Point" || !geometry.contains("coordinates") || !geometry["coordinates"].is_array()) {
        return false;
    }

    float lon = geometry["coordinates"][0];
    float lat = geometry["coordinates"][1];
    sf::Vector2f projectedPos = project({ lon, lat });

    cities.push_back({ name, projectedPos, category });
    return true;
}
