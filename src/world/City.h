#pragma once

#include <string>
#include <SFML/Graphics.hpp>

struct City {
    std::string name;
    sf::Vector2f position; // Position on the map
    int population;

    City(const std::string& name_, const sf::Vector2f& pos, int pop)
        : name(name_), position(pos), population(pop) {}
};
