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

// Define static constants for various map element colors
const sf::Color WorldMap::LAND_COLOR = sf::Color(231, 232, 234);
const sf::Color WorldMap::CITY_COLOR = sf::Color(236, 214, 214);
const sf::Color WorldMap::TOWN_COLOR = sf::Color(214, 214, 236);
const sf::Color WorldMap::SUBURB_COLOR = sf::Color(214, 236, 214);

// <summary>
// Constructor for WorldMap class. Initializes the file paths for GeoJSON data.
// </summary>
WorldMap::WorldMap(const std::string& geoJsonPath,
    const std::string& citiesGeoJsonPath,
    const std::string& townsGeoJsonPath,
    const std::string& suburbsGeoJsonPath)
    : geoJsonFilePath(geoJsonPath),
    citiesGeoJsonFilePath(citiesGeoJsonPath),
    townsGeoJsonFilePath(townsGeoJsonPath),
    suburbsGeoJsonFilePath(suburbsGeoJsonPath) {}

// Destructor for WorldMap class
WorldMap::~WorldMap() {}

// <summary>
// Initializes the WorldMap by loading GeoJSON data.
// Returns true if loading is successful, otherwise false.
// </summary>
bool WorldMap::Init() {
    if (!loadGeoJSON()) {
        std::cerr << "Failed to load GeoJSON data." << std::endl;
        return false;
    }

    return true;
}

// <summary>
// Loads GeoJSON data for land, cities, towns, and suburbs.
// Returns true if all data is loaded successfully, otherwise false.
// </summary>
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

// <summary>
// Processes a geometry object, determining its type (Polygon or MultiPolygon) and rendering it accordingly.
// </summary>
// <param name="geometry">JSON object representing the geometry</param>
// <param name="color">Color to render the geometry</param>
// <param name="targetShapes">Vector to store the generated shapes</param>
// <param name="name">Optional name of the place</param>
// <param name="category">Category of the place (e.g., City, Town)</param>
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

// <summary>
// Processes a Polygon geometry, converting coordinates to SFML vertex arrays.
// </summary>
// <param name="coordinates">JSON array of Polygon coordinates</param>
// <param name="color">Color to render the Polygon</param>
// <param name="targetShapes">Vector to store the generated shapes</param>
// <param name="name">Optional name of the place</param>
// <param name="category">Category of the place (e.g., City, Town)</param>
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

// <summary>
// Processes a MultiPolygon geometry by iteratively processing each Polygon.
// </summary>
// <param name="coordinates">JSON array of MultiPolygon coordinates</param>
// <param name="color">Color to render the MultiPolygon</param>
// <param name="targetShapes">Vector to store the generated shapes</param>
// <param name="name">Optional name of the place</param>
// <param name="category">Category of the place (e.g., City, Town)</param>
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

// <summary>
// Converts a Polygon into a filled and outlined VertexArray using Mapbox's Earcut algorithm.
// </summary>
// <param name="polygon">Vector of vector of SFML vectors representing the Polygon</param>
// <param name="color">Color to render the Polygon</param>
// <param name="targetShapes">Vector to store the generated shapes</param>
// <param name="name">Optional name of the place</param>
// <param name="category">Category of the place (e.g., City, Town)</param>
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
        area.bounds = filledVA.getBounds(); // Compute bounds here
        placeAreas.push_back(area);
    }
    else {
        // Store in targetShapes (e.g., landShapes)
        targetShapes.push_back(filledVA);
        targetShapes.push_back(outlineVA);
    }

    return true;
}

// <summary>
// Projects geographic longitude and latitude coordinates to map coordinates using a simple equirectangular projection.
// </summary>
// <param name="lonLat">Longitude and Latitude in sf::Vector2f</param>
sf::Vector2f WorldMap::project(const sf::Vector2f& lonLat) const {
    float x = (lonLat.x + 180.0f) / 360.0f * WORLD_WIDTH;
    float y = (90.0f - lonLat.y) / 180.0f * WORLD_HEIGHT;
    return { x, y };
}

// <summary>
// Renders the world map onto the given SFML window.
// Only draws shapes that are visible within the current camera view.
// </summary>
// <param name="window">SFML RenderWindow to draw to</param>
// <param name="camera">Camera view for determining visible area</param>
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
    for (const auto& shape : landShapes) {
        sf::FloatRect shapeBounds = shape.getBounds();
        if (cameraRect.intersects(shapeBounds)) {
            window.draw(shape);
        }
    }

    // Restore the original view
    window.setView(originalView);
}

// <summary>
// Gets the place areas (e.g., cities, towns) stored in the world map.
// </summary>
const std::vector<PlaceArea>& WorldMap::GetPlaceAreas() const {
    return placeAreas;
}

// <summary>
// Adds a station at the specified position.
// </summary>
bool WorldMap::AddStation(const sf::Vector2f& position) {
    stations.emplace_back(position);
    return true;
}

// <summary>
// Finds a station at a given position within a certain radius determined by the zoom level.
// </summary>
Station* WorldMap::GetStationAtPosition(const sf::Vector2f& position, float zoomLevel) {
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

// <summary>
// Adds a line to the world map.
// </summary>
void WorldMap::AddLine(const Line& line) {
    lines.push_back(line);
}

// <summary>
// Gets all stations stored in the world map.
// </summary>
const std::vector<Station>& WorldMap::GetStations() const {
    return stations;
}

// <summary>
// Gets all lines stored in the world map.
// </summary>
const std::vector<Line>& WorldMap::GetLines() const {
    return lines;
}

// <summary>
// Starts building a new line from a specified start position.
// </summary>
void WorldMap::StartBuildingLine(const sf::Vector2f& startPosition) {
    currentLine = std::make_unique<Line>();
    currentLine->AddNode(startPosition, true); // Pass 'true' to indicate it's a station
    isBuildingLine = true;
}

// <summary>
// Finishes the current line that is being built and adds it to the world map.
// </summary>
void WorldMap::FinishCurrentLine() {
    if (currentLine) {
        currentLine->SetActive(false);
        lines.push_back(*currentLine);
        currentLine.reset();
        isBuildingLine = false;
    }
}

// <summary>
// Gets the current line being built.
// </summary>
const Line* WorldMap::GetCurrentLine() const {
    return isBuildingLine ? currentLine.get() : nullptr;
}

// <summary>
// Checks if a line is currently being built.
// </summary>
bool WorldMap::IsBuildingLine() const {
    return isBuildingLine;
}

// <summary>
// Sets the current mouse position, used for line building.
// </summary>
void WorldMap::SetCurrentMousePosition(const sf::Vector2f& position) {
    currentMousePosition = position;
}

// <summary>
// Sets whether the next segment of the current line should be curved.
// </summary>
void WorldMap::SetNextSegmentCurved(bool curved) {
    isNextSegmentCurved = curved;
}

// <summary>
// Gets whether the next segment of the current line should be curved.
// </summary>
bool WorldMap::GetIsNextSegmentCurved() const {
    return isNextSegmentCurved;
}

// <summary>
// Adds a new node to the current line being built.
// </summary>
void WorldMap::AddNodeToCurrentLine(const sf::Vector2f& position, bool isStation) {
    if (currentLine && isBuildingLine) {
        currentLine->AddNode(position, isStation);
    }
}

// <summary>
// Sets the selected line in the world map.
// </summary>
void WorldMap::SetSelectedLine(Line* line) {
    selectedLine = line;
}

// <summary>
// Gets the selected line in the world map.
// </summary>
Line* WorldMap::GetSelectedLine() const {
    return selectedLine;
}

// <summary>
// Finds a line at the given position, considering the zoom level.
// </summary>
Line* WorldMap::GetLineAtPosition(const sf::Vector2f& position, float zoomLevel) {
    float tolerance = 5.0f * zoomLevel; // Adjust tolerance based on zoom level

    for (auto& line : lines) {
        const auto& splinePoints = line.GetSplinePoints();
        for (size_t i = 0; i < splinePoints.size() - 1; ++i) {
            sf::Vector2f p1 = splinePoints[i];
            sf::Vector2f p2 = splinePoints[i + 1];

            // Calculate the closest point on the segment to the position
            sf::Vector2f delta = p2 - p1;
            float segmentLengthSquared = delta.x * delta.x + delta.y * delta.y;
            float t = ((position - p1).x * delta.x + (position - p1).y * delta.y) / segmentLengthSquared;
            t = std::max(0.0f, std::min(1.0f, t));
            sf::Vector2f projection = p1 + t * delta;

            float distanceSquared = (position - projection).x * (position - projection).x +
                (position - projection).y * (position - projection).y;

            if (distanceSquared <= tolerance * tolerance) {
                return &line;
            }
        }
    }
    return nullptr;
}

// <summary>
// Gets all lines stored in the world map (modifiable).
// </summary>
std::vector<Line>& WorldMap::GetLines() {
    return lines;
}
