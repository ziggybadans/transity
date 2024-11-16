#include "WorldMap.h"
#include <iostream>

WorldMap::WorldMap(const std::string& geoJsonPath,
                   const std::string& citiesGeoJsonPath,
                   const std::string& townsGeoJsonPath,
                   const std::string& suburbsGeoJsonPath)
    : m_geoJsonFilePath(geoJsonPath)
    , m_citiesGeoJsonFilePath(citiesGeoJsonPath)
    , m_townsGeoJsonFilePath(townsGeoJsonPath)
    , m_suburbsGeoJsonFilePath(suburbsGeoJsonPath)
    , m_mapLoader(m_mapData)
{
}

bool WorldMap::Init() {
    if (!m_mapLoader.LoadGeoJSONFiles(m_geoJsonFilePath,
                                     m_citiesGeoJsonFilePath,
                                     m_townsGeoJsonFilePath,
                                     m_suburbsGeoJsonFilePath)) {
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
    const auto& landShapes = m_mapData.GetLandShapes();
    for (const auto& shape : landShapes) {
        sf::FloatRect shapeBounds = shape.getBounds();
        if (cameraRect.intersects(shapeBounds)) {
            window.draw(shape);
        }
    }

    window.setView(originalView);
}

const std::vector<PlaceArea>& WorldMap::GetPlaceAreas() const {
    return m_mapData.GetPlaceAreas();
}