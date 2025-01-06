#pragma once

#include <vector>
#include <list>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <memory>
#include <unordered_map>

#include "City.h"
#include "Line.h"
#include "Segment.h"
#include "HandleManager.h"
#include "SelectionManager.h"
#include "../core/StateManager.h"
#include "../managers/InputManager.h"
#include "../Constants.h"
#include "../Debug.h"
#include "../entity/Train.h"
#include <SFML/Graphics.hpp> // Assuming sf::Vector2f is from SFML

class Camera;

class Map {
public:
    // Constructor
    explicit Map(unsigned int size, StateManager& sm)
        : m_size(size),
        m_grid(size, std::vector<int>(size, 1)),
        m_minRadius(100),
        stateManager(sm) {}

    SelectionManager GetSelectionManager() { return selectionManager; }
    const SelectionManager& GetSelectionManager() const { return selectionManager; }

    // Selection Handling
    void SelectObject(const sf::Vector2f& pos);
    void DeselectAll();

    // Tile management
    void SetTile(unsigned int x, unsigned int y, int value);
    unsigned int GetSize() const { return m_size; }
    int GetTile(unsigned int x, unsigned int y) const {
        if (x >= m_size || y >= m_size)
            throw std::out_of_range("GetTile: Invalid tile coordinates");
        return m_grid[x][y];
    }

    // City management
    void AddCity(const sf::Vector2f pos);
    std::list<City>& GetCities() { return m_cities; }
    void SelectCity(City* city) { selectionManager.SelectCity(city); }
    bool SelectCity(sf::Vector2f pos);
    void DeselectCity() { selectionManager.DeselectAll(); }

    // Line management
    void UseLineMode(sf::Vector2f pos);
    void CreateLine(sf::Vector2f pos);
    void AddToLineStart(sf::Vector2f pos);
    void AddToLineEnd(sf::Vector2f pos);
    void SelectLine(Line* line) { selectionManager.SelectLine(line); }
    bool SelectLine(sf::Vector2f pos);
    bool SelectLineHandle(sf::Vector2f pos);
    void DeselectLine() { selectionManager.DeselectAll(); }
    void RemoveLine();
    std::list<Line>& GetLines() { return m_lines; }
    Line* GetSelectedLine() const { return selectionManager.GetSelectedLine(); }
    void MoveSelectedLineHandle(sf::Vector2f newPos);
    void UpdateSharedSegments();
    std::vector<Segment> GetSharedSegments() const { return sharedSegments; }
    bool isLineSelected() { return selectionManager.GetSelectedLine() == nullptr ? false : true; }

    // Train management
    void UseTrainPlaceMode(sf::Vector2f pos, bool left);
    void AddTrain();
    void SelectTrain(Train* train) { selectionManager.SelectTrain(train); }
    bool SelectTrain(sf::Vector2f pos);
    void DeselectTrain() { selectionManager.DeselectAll(); }
    void RemoveTrain();
    Train* GetSelectedTrain() const { return selectionManager.GetSelectedTrain(); }
    std::vector<std::unique_ptr<Train>>& GetTrains() { return m_trains; }
    City* GetStartCityForTrain() const { return startCityForTrain; }
    City* GetEndCityForTrain() const { return endCityForTrain; }

private:
    // Grid representation
    std::vector<std::vector<int>> m_grid;
    unsigned int m_size;

    unsigned int m_minRadius;

    // Helper functions
    void Resize(unsigned int newSize);
    City* FindCityAtPosition(sf::Vector2f pos);
    float DistancePointToSegment(const sf::Vector2f& point, const sf::Vector2f& segStart, const sf::Vector2f& segEnd);
    std::pair<int, int> NormalizeSegment(int startIndex, int endIndex) const;
    std::string GenerateSegmentKey(const sf::Vector2f& start, const sf::Vector2f& end) const;
    static bool ArePositionsEqual(const sf::Vector2f& pos1, const sf::Vector2f& pos2, float epsilon = 0.1f);

    // Collections
    std::list<City> m_cities;
    std::list<Line> m_lines;
    std::vector<std::unique_ptr<Train>> m_trains;

    std::vector<Segment> sharedSegments; // List of shared segments
    City* startCityForTrain = nullptr;
    City* endCityForTrain = nullptr;

    // Selection Manager
    SelectionManager selectionManager;
    StateManager& stateManager;
};
