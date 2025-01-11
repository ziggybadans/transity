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
#include "managers/HandleManager.h"
#include "managers/SelectionManager.h"
#include "managers/CityManager.h"
#include "managers/LineManager.h"
#include "managers/TrainManager.h"
#include "../core/StateManager.h"
#include "../core/InputManager.h"
#include "../Constants.h"
#include "../Debug.h"
#include "../entity/Train.h"
#include <SFML/Graphics.hpp> // Assuming sf::Vector2f is from SFML
#include <nlohmann/json.hpp>

class Camera;

class Map {
public:
    // Constructor
    explicit Map(unsigned int size, StateManager& sm)
        : m_size(size),
        m_grid(size, std::vector<int>(size, 1)),
        m_minRadius(100),
        stateManager(sm), selectionManager(*this), cityManager(m_minRadius, *this), lineManager(*this), trainManager(*this, stateManager) {}

    nlohmann::json Serialize();
    void Deserialize(const nlohmann::json& j);

    // Tile management
    void SetTile(unsigned int x, unsigned int y, int value);
    unsigned int GetSize() const { return m_size; }
    int GetTile(unsigned int x, unsigned int y) const {
        if (x >= m_size || y >= m_size)
            throw std::out_of_range("GetTile: Invalid tile coordinates");
        return m_grid[x][y];
    }

    // Nodes
    void AddGenericNode(sf::Vector2f pos);
    void RemoveNode();
    void MoveNode(sf::Vector2f& newPos);
    std::list<Node>& GetNodes() { return m_nodes; }

    // City management
    std::list<City>& GetCities() { return cityManager.GetCities(); }
    City* GetSelectedCity() { return selectionManager.GetSelectedCity(); }
    void RemoveCity(City* city) { cityManager.RemoveCity(city); }
    void AddCity(const sf::Vector2f& pos) { cityManager.AddCity(pos); }
    void MoveCity(const sf::Vector2f& pos) { cityManager.MoveCity(pos); }

    // Line management
    std::list<Line>& GetLines() { return lineManager.GetLines(); }
    std::vector<Segment> GetSharedSegments() const { return lineManager.GetSharedSegments(); }
    bool isLineSelected() { return selectionManager.GetSelectedLine() == nullptr ? false : true; }
    Line* GetSelectedLine() { return selectionManager.GetSelectedLine(); }
    void SelectLine(Line* line) { selectionManager.SelectLine(line); }
    void RemoveLine() { lineManager.RemoveLine(); }
    void UseLineMode(const sf::Vector2f& pos) { lineManager.UseLineMode(pos); }
    void MoveSelectedLineHandle(const sf::Vector2f& pos) { lineManager.MoveSelectedLineHandle(pos); }

    // Train management
    std::vector<std::unique_ptr<Train>>& GetTrains() { return trainManager.GetTrains(); }
    City* GetStartCityForTrain() const { return trainManager.startCityForTrain; }
    City* GetEndCityForTrain() const { return trainManager.endCityForTrain; }
    Train* GetSelectedTrain() const { return selectionManager.GetSelectedTrain(); }
    void AddTrain() { trainManager.AddTrain(); }
    void RemoveTrain() { trainManager.RemoveTrain(); }
    void UseTrainPlaceMode(const sf::Vector2f& pos, bool left) { trainManager.UseTrainPlaceMode(pos, left); }

    // Passenger management
    int GetScore() const { return m_score; }
    void SetScore(int value) { m_score = value; }
    void UpdatePassengers(float dt) { cityManager.UpdatePassengers(dt); }
    void SpawnPassenger(City* origin, City* destination) { cityManager.SpawnPassenger(origin, destination); }

    // Helper methods
    float DistancePointToSegment(const sf::Vector2f& point, const sf::Vector2f& segStart, const sf::Vector2f& segEnd);
    std::vector<Node*> FindRouteBetweenNodes(Node* start, Node* end) { return lineManager.FindRouteBetweenNodes(start, end); }
    City* FindCityAtPosition(sf::Vector2f pos);
    std::pair<int, int> NormalizeSegment(int startIndex, int endIndex) const;
    std::string GenerateSegmentKey(const sf::Vector2f& start, const sf::Vector2f& end) const;
    bool ArePositionsEqual(const sf::Vector2f& pos1, const sf::Vector2f& pos2, float epsilon = 0.1f);
    Node* FindGenericNodeAtPosition(sf::Vector2f pos);
    bool WouldCauseParallelConflict(const sf::Vector2f& segStart, const sf::Vector2f& segEnd);
    void DeselectAll() { selectionManager.DeselectAll(); }
    void SelectObject(const sf::Vector2f& pos) { selectionManager.SelectObject(pos); }

private:
    // Grid representation
    std::vector<std::vector<int>> m_grid;
    unsigned int m_size;

    unsigned int m_minRadius;
    int m_score = 0;

    // Collections
    std::list<Node> m_nodes;

    // Selection Manager
    SelectionManager selectionManager;
    CityManager cityManager;
    StateManager& stateManager;
    LineManager lineManager;
    TrainManager trainManager;
};
