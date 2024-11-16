#include "MapData.h"

// Define static color constants
const sf::Color MapData::LAND_COLOR = sf::Color(231, 232, 234);
const sf::Color MapData::CITY_COLOR = sf::Color(236, 214, 214);
const sf::Color MapData::TOWN_COLOR = sf::Color(214, 214, 236);
const sf::Color MapData::SUBURB_COLOR = sf::Color(214, 236, 214);

MapData::MapData() {}

MapData::~MapData() {}

const std::vector<sf::VertexArray>& MapData::GetLandShapes() const {
    return m_landShapes;
}

const std::vector<PlaceArea>& MapData::GetPlaceAreas() const {
    return m_placeAreas;
}

void MapData::AddLandShape(const sf::VertexArray& shape) {
    m_landShapes.push_back(shape);
}

void MapData::AddPlaceArea(const PlaceArea& area) {
    m_placeAreas.push_back(area);
}
