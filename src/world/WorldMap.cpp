#include "WorldMap.h"
#include <iostream>
#include <fstream>
#include <cmath>
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

WorldMap::WorldMap(const std::string& geoJsonPath)
    : geoJsonFilePath(geoJsonPath), zoomLevelToSwitch(1.0f) // Adjust zoomLevelToSwitch as needed
{}

WorldMap::~WorldMap() {}

bool WorldMap::Init() {
    if (!loadGeoJSON()) {
        std::cerr << "Failed to load GeoJSON data." << std::endl;
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
        inFile.close();
        return false;
    }
    inFile.close();

    // Check if "geometries" exists and is an array
    if (!geoData.contains("geometries") || !geoData["geometries"].is_array()) {
        std::cerr << "Invalid GeoJSON structure: Missing 'geometries' array." << std::endl;
        return false;
    }

    const auto& geometries = geoData["geometries"];

    for (const auto& geometry : geometries) {
        if (!geometry.contains("type") || !geometry.contains("coordinates")) {
            std::cerr << "Invalid geometry object: Missing 'type' or 'coordinates'." << std::endl;
            continue; // Skip invalid geometry
        }

        std::string geometryType = geometry["type"];
        if (geometryType == "Polygon") {
            const auto& coordinates = geometry["coordinates"];
            if (!coordinates.is_array()) {
                std::cerr << "Invalid Polygon coordinates." << std::endl;
                continue;
            }

            // Convert Polygon to a vector of rings, each ring is a vector of sf::Vector2f
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
                    sf::Vector2f projectedPoint = project(sf::Vector2f(lon, lat));
                    ringPoints.push_back(projectedPoint);
                }

                if (!ringPoints.empty()) {
                    polygon.push_back(ringPoints);
                }
            }

            if (!polygon.empty()) {
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
                        va[i].color = sf::Color(34, 139, 34); // ForestGreen color for land
                    }
                    else {
                        std::cerr << "Index out of bounds during triangulation." << std::endl;
                    }
                }
                landShapes.push_back(va);
            }

        }
        else if (geometryType == "MultiPolygon") {
            const auto& multiPolygons = geometry["coordinates"];
            if (!multiPolygons.is_array()) {
                std::cerr << "Invalid MultiPolygon coordinates." << std::endl;
                continue;
            }

            for (const auto& polygons : multiPolygons) {
                if (!polygons.is_array()) {
                    std::cerr << "Invalid Polygon in MultiPolygon." << std::endl;
                    continue;
                }

                std::vector<std::vector<sf::Vector2f>> polygon;
                for (const auto& ring : polygons) {
                    if (!ring.is_array()) {
                        std::cerr << "Invalid ring in MultiPolygon." << std::endl;
                        continue;
                    }

                    std::vector<sf::Vector2f> ringPoints;
                    for (const auto& point : ring) {
                        if (!point.is_array() || point.size() < 2) {
                            std::cerr << "Invalid point in MultiPolygon ring." << std::endl;
                            continue;
                        }

                        float lon = point[0];
                        float lat = point[1];
                        sf::Vector2f projectedPoint = project(sf::Vector2f(lon, lat));
                        ringPoints.push_back(projectedPoint);
                    }

                    if (!ringPoints.empty()) {
                        polygon.push_back(ringPoints);
                    }
                }

                if (!polygon.empty()) {
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
                            va[i].color = sf::Color(34, 139, 34); // ForestGreen color for land
                        }
                        else {
                            std::cerr << "Index out of bounds during triangulation." << std::endl;
                        }
                    }
                    landShapes.push_back(va);
                }
            }
        }
        else {
            std::cerr << "Unsupported geometry type: " << geometryType << std::endl;
            // You can add handling for other geometry types if necessary
        }
    }

    // Optional: Log the number of shapes loaded
    std::cout << "Loaded " << landShapes.size() << " land shapes from GeoJSON." << std::endl;

    return true;
}

sf::Vector2f WorldMap::project(const sf::Vector2f& lonLat) const {
    // Simple equirectangular projection
    float x = (lonLat.x + 180.0f) / 360.0f * WORLD_WIDTH;
    float y = (90.0f - lonLat.y) / 180.0f * WORLD_HEIGHT;
    return sf::Vector2f(x, y);
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
