#pragma once
#include <list>
#include <SFML/Graphics.hpp>

class City;
class Map;

class CityManager {
public:
    explicit CityManager(unsigned int minRadius, Map& map)
        : m_minRadius(minRadius), m_map(map) {}

    void AddCity(const sf::Vector2f& pos);
    void RemoveCity(City* city);
    void MoveCity(const sf::Vector2f& newPos);
    std::list<City>& GetCities() { return m_cities; }

    void SpawnPassenger(City* origin, City* destination);
    void UpdatePassengers(float dt);

private:
    std::list<City> m_cities;
    unsigned int m_minRadius;
    Map& m_map;

    // Helper functions, collision checks etc. can be placed here
};
