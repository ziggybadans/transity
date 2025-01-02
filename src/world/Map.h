// Map.h
#pragma once

#include <vector>
#include <list>
#include <algorithm>
#include <stdexcept>
#include <string>

#include "City.h"
#include "Line.h"
#include "../Constants.h"
#include "../Debug.h"
#include "../entity/Train.h"
#include <SFML/Graphics.hpp> // Assuming sf::Vector2f is from SFML

class Map {
public:
    // Constructor
    Map(unsigned int size);

    // Tile management
    void SetTile(unsigned int x, unsigned int y, int value);
    int GetSize() const;
    int GetTile(int x, int y) const;

    // City management
    void AddCity(sf::Vector2f pos);

    // Line management
    void UseLineMode(sf::Vector2f pos);
    void CreateLine(sf::Vector2f pos);
    void AddToLine(sf::Vector2f pos);
    void SelectLine(Line* line);
    void SelectLine(sf::Vector2f pos);
    void DeselectLine();

    // Train management
    void AddTrain();
    void SelectTrain(Train* train);
    void SelectTrain(sf::Vector2f pos);

    // Public members
    std::list<Train> m_trains; // Container of trains
    std::list<City> m_cities;
    std::list<Line> m_lines;
    Line* selectedLine;
    Train* selectedTrain;

private:
    // Grid representation
    std::vector<std::vector<int>> m_grid;
    unsigned int m_size;

    unsigned int m_minRadius;

    // Helper functions
    void Resize(unsigned int newSize);
};
