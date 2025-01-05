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
    Map(unsigned int size)
        : m_size(size),
        m_grid(size, std::vector<int>(size, 1)),
        m_minRadius(100),
        selectedLine(nullptr),
        selectedTrain(nullptr) {}

    void SelectObject(sf::Vector2f pos);
    void DeselectAll();

    // Tile management
    void SetTile(unsigned int x, unsigned int y, int value);
    int GetSize() const { return m_size; }
    int GetTile(int x, int y) const { return m_grid[x][y]; }

    // City management
    void AddCity(sf::Vector2f pos);
    std::list<City>& GetCities() { return m_cities; }
    void SelectCity(City* city);
    bool SelectCity(sf::Vector2f pos);
    void DeselectCity();

    // Line management
    void UseLineMode(sf::Vector2f pos);
    void CreateLine(sf::Vector2f pos);
    void AddToLineStart(sf::Vector2f pos);
    void AddToLineEnd(sf::Vector2f pos);
    void SelectLine(Line* line);
    bool SelectLine(sf::Vector2f pos);
    bool SelectLineHandle(sf::Vector2f pos);
    void DeselectLine();
    void RemoveLine();
    std::list<Line>& GetLines() { return m_lines; }
    Line* GetSelectedLine() { return selectedLine; }
    void MoveSelectedLineHandle(sf::Vector2f newPos);

    // Train management
    void AddTrain();
    void SelectTrain(Train* train);
    bool SelectTrain(sf::Vector2f pos);
    void DeselectTrain();
    void RemoveTrain();
    Train* GetSelectedTrain() { return selectedTrain; }
    std::vector<std::unique_ptr<Train>>& GetTrains() { return m_trains; }

private:
    // Grid representation
    std::vector<std::vector<int>> m_grid;
    unsigned int m_size;

    unsigned int m_minRadius;

    // Helper functions
    void Resize(unsigned int newSize);
    City* FindCityAtPosition(sf::Vector2f pos);
    float DistancePointToSegment(const sf::Vector2f& point, const sf::Vector2f& segStart, const sf::Vector2f& segEnd);

    // Public members
    std::list<City> m_cities;
    std::list<Line> m_lines;
    std::vector<std::unique_ptr<Train>> m_trains;
    City* selectedCity;
    Line* selectedLine;
    Train* selectedTrain;
};
