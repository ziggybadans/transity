#include "MapLoader.h"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

// Mapbox Earcut namespace specialization for SFML's sf::Vector2f
namespace mapbox
{
    namespace util
    {
        template <>
        struct nth<0, sf::Vector2f>
        {
            static auto get(const sf::Vector2f &t)
            {
                return t.x;
            }
        };
        template <>
        struct nth<1, sf::Vector2f>
        {
            static auto get(const sf::Vector2f &t)
            {
                return t.y;
            }
        };
    }
}

MapLoader::MapLoader(MapData &mapData)
    : m_mapData(mapData) {}

MapLoader::~MapLoader() {}

bool MapLoader::LoadGeoJSONFiles(
    const std::string &landGeoJsonPath,
    const std::string &citiesGeoJsonPath,
    const std::string &townsGeoJsonPath,
    const std::string &suburbsGeoJsonPath)
{
    std::cout << "Loading land GeoJSON from: " << landGeoJsonPath << std::endl;
    if (!LoadGeoJSON(landGeoJsonPath, MapData::LAND_COLOR))
    {
        std::cerr << "Failed to load land GeoJSON." << std::endl;
        return false;
    }

    if (!LoadGeoJSON(citiesGeoJsonPath, MapData::CITY_COLOR, PlaceCategory::City))
    {
        std::cerr << "Failed to load cities GeoJSON data." << std::endl;
        return false;
    }

    if (!LoadGeoJSON(townsGeoJsonPath, MapData::TOWN_COLOR, PlaceCategory::Town))
    {
        std::cerr << "Failed to load towns GeoJSON data." << std::endl;
        return false;
    }

    if (!LoadGeoJSON(suburbsGeoJsonPath, MapData::SUBURB_COLOR, PlaceCategory::Suburb))
    {
        std::cerr << "Failed to load suburbs GeoJSON data." << std::endl;
        return false;
    }

    return true;
}

bool MapLoader::LoadGeoJSON(const std::string &geoJsonFilePath, const sf::Color &color, PlaceCategory category)
{
    std::ifstream geoJsonFile(geoJsonFilePath);
    if (!geoJsonFile.is_open())
    {
        std::cerr << "Could not open GeoJSON file: " << geoJsonFilePath << std::endl;
        return false;
    }

    json geoData;
    try
    {
        geoJsonFile >> geoData;
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "JSON Parsing error: " << e.what() << std::endl;
        return false;
    }

    if (geoData.contains("geometries") && geoData["geometries"].is_array())
    {
        // For land data
        const auto &geometries = geoData["geometries"];
        for (const auto &geometry : geometries)
        {
            if (!ProcessGeometry(geometry, color))
            {
                std::cerr << "Failed to process geometry." << std::endl;
            }
        }
    }
    else if (geoData.contains("features") && geoData["features"].is_array())
    {
        // For cities, towns, suburbs
        const auto &features = geoData["features"];
        for (const auto &feature : features)
        {
            if (!feature.contains("geometry") || !feature["geometry"].is_object())
            {
                continue;
            }
            std::string name = feature["properties"].value("name", "Unnamed Place");
            ProcessGeometry(feature["geometry"], color, name, category);
        }
    }
    else
    {
        std::cerr << "Invalid GeoJSON structure: Missing 'geometries' or 'features' array." << std::endl;
        return false;
    }

    return true;
}

bool MapLoader::ProcessGeometry(const json &geometry, const sf::Color &color, const std::string &name, PlaceCategory category)
{
    if (!geometry.contains("type") || !geometry.contains("coordinates"))
    {
        std::cerr << "Invalid geometry object: Missing 'type' or 'coordinates'." << std::endl;
        return false;
    }

    std::string geometryType = geometry["type"];
    const auto &coordinates = geometry["coordinates"];

    if (geometryType == "Polygon")
    {
        return ProcessPolygon(coordinates, color, name, category);
    }
    else if (geometryType == "MultiPolygon")
    {
        return ProcessMultiPolygon(coordinates, color, name, category);
    }
    else
    {
        std::cerr << "Unsupported geometry type: " << geometryType << std::endl;
        return false;
    }
}

bool MapLoader::ProcessPolygon(const json &coordinates, const sf::Color &color, const std::string &name, PlaceCategory category)
{
    if (!coordinates.is_array())
    {
        std::cerr << "Invalid Polygon coordinates." << std::endl;
        return false;
    }

    std::vector<std::vector<sf::Vector2f>> polygon;
    for (const auto &ring : coordinates)
    {
        if (!ring.is_array())
        {
            std::cerr << "Invalid ring in Polygon." << std::endl;
            continue;
        }

        std::vector<sf::Vector2f> ringPoints;
        for (const auto &point : ring)
        {
            if (!point.is_array() || point.size() < 2)
            {
                std::cerr << "Invalid point in Polygon ring." << std::endl;
                continue;
            }

            float lon = point[0];
            float lat = point[1];
            ringPoints.push_back(Project({lon, lat}));
        }

        if (!ringPoints.empty())
        {
            polygon.push_back(ringPoints);
        }
    }

    return !polygon.empty() && CreateVertexArrayFromPolygon(polygon, color, name, category);
}

bool MapLoader::ProcessMultiPolygon(const json &coordinates, const sf::Color &color, const std::string &name, PlaceCategory category)
{
    if (!coordinates.is_array())
    {
        std::cerr << "Invalid MultiPolygon coordinates." << std::endl;
        return false;
    }

    for (const auto &polygons : coordinates)
    {
        if (!ProcessPolygon(polygons, color, name, category))
        {
            std::cerr << "Failed to process Polygon in MultiPolygon." << std::endl;
        }
    }

    return true;
}

bool MapLoader::CreateVertexArrayFromPolygon(
    const std::vector<std::vector<sf::Vector2f>> &polygon,
    const sf::Color &color,
    const std::string &name,
    PlaceCategory category)
{
    std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);

    std::vector<sf::Vector2f> flattenedPoints;
    for (const auto &ring : polygon)
    {
        flattenedPoints.insert(flattenedPoints.end(), ring.begin(), ring.end());
    }

    sf::VertexArray filledVA(sf::Triangles, indices.size());
    for (size_t i = 0; i < indices.size(); ++i)
    {
        if (indices[i] < flattenedPoints.size())
        {
            filledVA[i].position = flattenedPoints[indices[i]];
            filledVA[i].color = color;
        }
        else
        {
            std::cerr << "Index out of bounds during triangulation." << std::endl;
            return false;
        }
    }

    sf::VertexArray outlineVA(sf::LineStrip);
    for (const auto &ring : polygon)
    {
        for (const auto &point : ring)
        {
            outlineVA.append(sf::Vertex(point, sf::Color::Black));
        }
        if (!ring.empty())
        {
            outlineVA.append(sf::Vertex(ring.front(), sf::Color::Black));
        }
    }

    if (category != PlaceCategory::Unknown && !name.empty())
    {
        PlaceArea area;
        area.name = name;
        area.category = category;
        area.filledShape = filledVA;
        area.outline = outlineVA;
        area.bounds = filledVA.getBounds();
        m_mapData.AddPlaceArea(area);
    }
    else
    {
        m_mapData.AddLandShape(filledVA);
        m_mapData.AddLandShape(outlineVA);
        std::cout << "Added land shape to MapData." << std::endl;
    }

    return true;
}

sf::Vector2f MapLoader::Project(const sf::Vector2f &lonLat) const
{
    float x = (lonLat.x + 180.0f) / 360.0f * WORLD_WIDTH;
    float y = (90.0f - lonLat.y) / 180.0f * WORLD_HEIGHT;
    return {x, y};
}
