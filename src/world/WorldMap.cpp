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
const sf::Color WorldMap::LAND_COLOR = sf::Color(34, 139, 34); // ForestGreen
const sf::Color WorldMap::CITY_COLOR = sf::Color::Red;
const sf::Color WorldMap::TOWN_COLOR = sf::Color::Blue;
const sf::Color WorldMap::SUBURB_COLOR = sf::Color::Green;

WorldMap::WorldMap(const std::string& geoJsonPath,
    const std::string& citiesGeoJsonPath,
    const std::string& townsGeoJsonPath,
    const std::string& suburbsGeoJsonPath)
    : geoJsonFilePath(geoJsonPath),
    citiesGeoJsonFilePath(citiesGeoJsonPath),
    townsGeoJsonFilePath(townsGeoJsonPath),
    suburbsGeoJsonFilePath(suburbsGeoJsonPath) {}

WorldMap::~WorldMap() {}

bool WorldMap::Init() {
    if (!loadGeoJSON()) {
        std::cerr << "Failed to load GeoJSON data." << std::endl;
        return false;
    }

    return true;
}

bool WorldMap::loadGeoJSON() {
    // Load land shapes
    std::ifstream landFile(geoJsonFilePath);
    if (!landFile.is_open()) {
        std::cerr << "Could not open GeoJSON file: " << geoJsonFilePath << std::endl;
        return false;
    }

    json landData;
    try {
        landFile >> landData;
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON Parsing error: " << e.what() << std::endl;
        return false;
    }

    if (!landData.contains("geometries") || !landData["geometries"].is_array()) {
        std::cerr << "Invalid GeoJSON structure: Missing 'geometries' array." << std::endl;
        return false;
    }

    const auto& geometries = landData["geometries"];
    for (const auto& geometry : geometries) {
        if (!processGeometry(geometry, LAND_COLOR, landShapes)) {
            std::cerr << "Failed to process geometry." << std::endl;
        }
    }

    // Load cities
    std::ifstream citiesFile(citiesGeoJsonFilePath);
    if (!citiesFile.is_open()) {
        std::cerr << "Could not open cities GeoJSON file: " << citiesGeoJsonFilePath << std::endl;
        return false;
    }

    json citiesData;
    try {
        citiesFile >> citiesData;
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON Parsing error in cities GeoJSON: " << e.what() << std::endl;
        return false;
    }

    if (!citiesData.contains("features") || !citiesData["features"].is_array()) {
        std::cerr << "Invalid cities GeoJSON structure: Missing 'features' array." << std::endl;
        return false;
    }

    const auto& cityFeatures = citiesData["features"];
    for (const auto& feature : cityFeatures) {
        if (!feature.contains("geometry") || !feature["geometry"].is_object()) {
            continue;
        }
        std::string name = feature["properties"].value("name", "Unnamed City");
        processGeometry(feature["geometry"], CITY_COLOR, landShapes, name, PlaceCategory::City);
    }

    // Load towns
    std::ifstream townsFile(townsGeoJsonFilePath);
    if (!townsFile.is_open()) {
        std::cerr << "Could not open towns GeoJSON file: " << townsGeoJsonFilePath << std::endl;
        return false;
    }

    json townsData;
    try {
        townsFile >> townsData;
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON Parsing error in towns GeoJSON: " << e.what() << std::endl;
        return false;
    }

    if (!townsData.contains("features") || !townsData["features"].is_array()) {
        std::cerr << "Invalid towns GeoJSON structure: Missing 'features' array." << std::endl;
        return false;
    }

    const auto& townFeatures = townsData["features"];
    for (const auto& feature : townFeatures) {
        if (!feature.contains("geometry") || !feature["geometry"].is_object()) {
            continue;
        }
        std::string name = feature["properties"].value("name", "Unnamed Town");
        processGeometry(feature["geometry"], TOWN_COLOR, landShapes, name, PlaceCategory::Town);
    }

    // Load suburbs
    std::ifstream suburbsFile(suburbsGeoJsonFilePath);
    if (!suburbsFile.is_open()) {
        std::cerr << "Could not open suburbs GeoJSON file: " << suburbsGeoJsonFilePath << std::endl;
        return false;
    }

    json suburbsData;
    try {
        suburbsFile >> suburbsData;
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON Parsing error in suburbs GeoJSON: " << e.what() << std::endl;
        return false;
    }

    if (!suburbsData.contains("features") || !suburbsData["features"].is_array()) {
        std::cerr << "Invalid suburbs GeoJSON structure: Missing 'features' array." << std::endl;
        return false;
    }

    const auto& suburbFeatures = suburbsData["features"];
    for (const auto& feature : suburbFeatures) {
        if (!feature.contains("geometry") || !feature["geometry"].is_object()) {
            continue;
        }
        std::string name = feature["properties"].value("name", "Unnamed Suburb");
        processGeometry(feature["geometry"], SUBURB_COLOR, landShapes, name, PlaceCategory::Suburb);
    }

    return true;
}

bool WorldMap::processGeometry(const json& geometry, const sf::Color& color, std::vector<sf::VertexArray>& targetShapes, const std::string& name, PlaceCategory category) {
    if (!geometry.contains("type") || !geometry.contains("coordinates")) {
        std::cerr << "Invalid geometry object: Missing 'type' or 'coordinates'." << std::endl;
        return false;
    }

    std::string geometryType = geometry["type"];
    const auto& coordinates = geometry["coordinates"];

    if (geometryType == "Polygon") {
        return processPolygon(coordinates, color, targetShapes, name, category);
    }
    else if (geometryType == "MultiPolygon") {
        return processMultiPolygon(coordinates, color, targetShapes, name, category);
    }
    else {
        std::cerr << "Unsupported geometry type: " << geometryType << std::endl;
        return false;
    }
}

bool WorldMap::processPolygon(const json& coordinates, const sf::Color& color, std::vector<sf::VertexArray>& targetShapes, const std::string& name, PlaceCategory category) {
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

    return !polygon.empty() && createVertexArrayFromPolygon(polygon, color, targetShapes, name, category);
}

bool WorldMap::processMultiPolygon(const json& coordinates, const sf::Color& color, std::vector<sf::VertexArray>& targetShapes, const std::string& name, PlaceCategory category) {
    if (!coordinates.is_array()) {
        std::cerr << "Invalid MultiPolygon coordinates." << std::endl;
        return false;
    }

    for (const auto& polygons : coordinates) {
        if (!processPolygon(polygons, color, targetShapes, name, category)) {
            std::cerr << "Failed to process Polygon in MultiPolygon." << std::endl;
        }
    }

    return true;
}

bool WorldMap::createVertexArrayFromPolygon(
    const std::vector<std::vector<sf::Vector2f>>& polygon,
    const sf::Color& color,
    std::vector<sf::VertexArray>& targetShapes,
    const std::string& name,
    PlaceCategory category
) {
    // Triangulate the polygon using Earcut
    std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);

    // Flatten the polygon points
    std::vector<sf::Vector2f> flattenedPoints;
    for (const auto& ring : polygon) {
        flattenedPoints.insert(flattenedPoints.end(), ring.begin(), ring.end());
    }

    // Create a filled VertexArray
    sf::VertexArray filledVA(sf::Triangles, indices.size());
    for (size_t i = 0; i < indices.size(); ++i) {
        if (indices[i] < flattenedPoints.size()) {
            filledVA[i].position = flattenedPoints[indices[i]];
            filledVA[i].color = color;
        }
        else {
            std::cerr << "Index out of bounds during triangulation." << std::endl;
            return false;
        }
    }

    // Create an outline VertexArray using LineStrip
    sf::VertexArray outlineVA(sf::LineStrip);
    for (const auto& ring : polygon) {
        for (const auto& point : ring) {
            outlineVA.append(sf::Vertex(point, sf::Color::Black)); // Outline color (e.g., Black)
        }
        // Close the loop by connecting the last point to the first
        if (!ring.empty()) {
            outlineVA.append(sf::Vertex(ring.front(), sf::Color::Black));
        }
    }

    if (category != PlaceCategory::Unknown && !name.empty()) {
        // Store in placeAreas
        PlaceArea area;
        area.name = name;
        area.category = category;
        area.filledShape = filledVA;
        area.outline = outlineVA;
        placeAreas.push_back(area);
    }
    else {
        // Store in targetShapes (e.g., landShapes)
        targetShapes.push_back(filledVA);
        targetShapes.push_back(outlineVA);
    }

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

    // Draw all place area filled shapes
    for (const auto& area : placeAreas) {
        window.draw(area.filledShape);
    }

    // Draw all place area outlines
    for (const auto& area : placeAreas) {
        window.draw(area.outline);
    }

    // Restore the original view
    window.setView(originalView);
}

const std::vector<PlaceArea>& WorldMap::GetPlaceAreas() const {
    return placeAreas;
}
