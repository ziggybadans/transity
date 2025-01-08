#pragma once
#include "Node.h"

class City : public Node {
public:
    City(const std::string& cityName, const sf::Vector2f& cityPosition,
        unsigned int cityPopulation, float cityRadius = 10.0f)
        : Node(cityName, cityPosition, cityRadius),
        population(cityPopulation) {}

    unsigned int GetPopulation() const { return population; }

    bool operator==(const City& other) const {
        return GetName() == other.GetName() && GetPosition() == other.GetPosition();
    }

private:
    unsigned int population;
};