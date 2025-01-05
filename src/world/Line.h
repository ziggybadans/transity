// Line.h
#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <SFML/Graphics.hpp>

#include "City.h"

class Train;

// Structure representing a point on the line
struct LinePoint {
    bool isCity;             // True if it represents a city, false if it's a node
    sf::Vector2f position;   // The world position of the city or node
    City* city;              // Pointer to a City if isCity == true, otherwise nullptr

    LinePoint(bool cityFlag = false, sf::Vector2f pos = sf::Vector2f(0.f, 0.f), City* c = nullptr)
        : isCity(cityFlag), position(pos), city(c) {}
};

// Structure representing a handle for manipulation (e.g., in a UI)
struct Handle {
    int index;         // Index of the node in the points vector
    bool isSelected;   // Selection state

    Handle(int idx = 0, bool selected = false) : index(idx), isSelected(selected) {}
};

// The Line class represents a railway line connecting cities and nodes
class Line {
public:
    // Constructor
    Line(City* startCity, const std::string& lineName,
        const sf::Color& lineColor = sf::Color::Blue, float lineThickness = 4.0f);

    // Destructor
    ~Line();

    // City management
    void AddCityToStart(City* city);
    void AddCityToEnd(City* city);
    void InsertCityAfter(int index, City* city);
    const std::vector<City*> GetCities() const;

    // Node management
    void AddNode(sf::Vector2f pos);

    // Getters
    const std::vector<LinePoint>& GetPoints() const { return points; }
    std::vector<sf::Vector2f> GetPathPoints() const;
    const std::vector<int> GetCityIndices() const;
    sf::Vector2f GetStartPosition() const;
    sf::Vector2f GetEndPosition() const;
    sf::Vector2f GetPointPosition(int index) const;

    // Selection management
    void SelectHandle(int index);
    void DeselectHandles();
    int GetSelectedHandleIndex() const;
    std::vector<Handle> GetHandles() const { return handles; }

    // Handle manipulation
    void MoveHandle(int index, sf::Vector2f newPos);

    // Train management
    void AddTrain(Train* train);
    void RemoveTrain(Train* train);
    bool HasTrains() const { return trains.empty(); }
    const std::vector<Train*>& GetTrains() const { return trains; }

    // Property setters and getters
    void SetThickness(float newThickness) { thickness = newThickness; }
    void SetSelected(bool value) { selected = value; }
    std::string GetName() const { return name; }
    sf::Color GetColor() const { return color; }
    float GetThickness() const { return thickness; }
    bool IsSelected() const { return selected; }

private:
    // Member variables
    std::vector<LinePoint> points;      // Points defining the line (cities and nodes)
    std::vector<Handle> handles;        // Handles for manipulation
    std::vector<Train*> trains;         // Trains traveling on the line
    std::string name;                   // Name of the line
    sf::Color color;                    // Color of the line
    float thickness;                    // Thickness of the line when drawn
    bool selected;                      // Selection state

    // Utility function to normalize a vector
    sf::Vector2f Normalize(const sf::Vector2f& vec) const;
};
