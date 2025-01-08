// City.h
#pragma once

#include <string>
#include <SFML/Graphics.hpp>

class City {
public:
    // Constructor
    City(const std::string& cityName,
        const sf::Vector2f& cityPosition,
        unsigned int cityPopulation,
        float cityRadius = 10.0f)
        : name(cityName),
        position(cityPosition),
        population(cityPopulation),
        radius(cityRadius),
        selected(false) {}

    // Getters
    std::string GetName() const { return name; }
    sf::Vector2f GetPosition() const { return position; }
    unsigned int GetPopulation() const { return population; }
    float GetRadius() const { return radius; }
    bool IsSelected() const { return selected; }

    // Setters
    void SetPosition(const sf::Vector2f& pos) { position = pos; }
    void SetSelected(bool value) { selected = value; }

    // Operator Overload
    bool operator==(const City& other) const {
        return name == other.name && position == other.position;
    }

private:
    std::string name;
    sf::Vector2f position;
    unsigned int population;
    float radius;
    bool selected;
};
