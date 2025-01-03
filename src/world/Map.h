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

    void SelectObject(sf::Vector2f pos);
    void DeselectAll();

    // Tile management
    void SetTile(unsigned int x, unsigned int y, int value);
    int GetSize() const;
    int GetTile(int x, int y) const;

    // City management
    void AddCity(sf::Vector2f pos);
    std::list<City>& GetCities() { return m_cities; }
    void SelectCity(City* city);
    bool SelectCity(sf::Vector2f pos);
    void DeselectCity();

    // Line management
    void UseLineMode(sf::Vector2f pos);
    void CreateLine(sf::Vector2f pos);
    void AddToLine(sf::Vector2f pos);
    void SelectLine(Line* line);
    bool SelectLine(sf::Vector2f pos);
    void DeselectLine();
    std::list<Line>& GetLines() { return m_lines; }

    // Train management
    void AddTrain();
    void SelectTrain(Train* train);
    bool SelectTrain(sf::Vector2f pos);
    void DeselectTrain();
    Train* GetSelectedTrain() { return selectedTrain; }
    std::list<Train>& GetTrains() { return m_trains; }

private:
    // Grid representation
    std::vector<std::vector<int>> m_grid;
    unsigned int m_size;

    unsigned int m_minRadius;

    // Helper functions
    void Resize(unsigned int newSize);
    float DistancePointToBezier(sf::Vector2f point, const BezierSegment& segment) const;
    sf::Vector2f ComputeCubicBezierPoint(const BezierSegment& segment, float t) const;

    // Public members
    std::list<City> m_cities;
    std::list<Line> m_lines;
    std::list<Train> m_trains;
    City* selectedCity;
    Line* selectedLine;
    Train* selectedTrain;
};
