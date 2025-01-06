#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <SFML/Graphics.hpp>

#include "City.h"
#include "HandleManager.h"
#include "Segment.h"

class Train;

struct LinePoint {
    bool isCity;             // True if it represents a city, false if it's a node
    sf::Vector2f position;   // The world position of the city or node
    City* city;              // Pointer to a City if isCity == true, otherwise nullptr

    LinePoint(bool cityFlag = false, sf::Vector2f pos = sf::Vector2f(0.f, 0.f), City* c = nullptr)
        : isCity(cityFlag), position(pos), city(c) {}
};

// Offset information per segment
struct OffsetInfo {
    sf::Vector2f offsetVector; // The perpendicular offset vector
    float transitionLength;    // Length over which to transition into/out of the offset
};

class Line {
public:
    // Constructor
    Line(City* startCity, const std::string& lineName,
        const sf::Color& lineColor = sf::Color::Blue, float lineThickness = 4.0f);

    // Destructor
    ~Line() = default;

    // City management
    void AddCityToStart(City* city);
    void AddCityToEnd(City* city);
    void InsertCityAfter(int index, City* city);
    std::vector<City*> GetCities() const;
    bool HasMultipleCities() { return GetCities().size() > 1; }

    // Node management
    void AddNode(sf::Vector2f pos);

    // New methods for offset management
    void CalculateOffsets(const std::vector<Segment>& sharedSegments);
    std::vector<sf::Vector2f> GetAdjustedPathPoints() const;

    // Getters
    const std::vector<LinePoint>& GetPoints() const { return points; }
    std::vector<sf::Vector2f> GetPathPoints() const;
    std::vector<int> GetCityIndices() const;
    sf::Vector2f GetStartPosition() const;
    sf::Vector2f GetEndPosition() const;
    sf::Vector2f GetPointPosition(int index) const;
    std::vector<int> GetIndicesBetweenCities(City* cityA, City* cityB) const;

    // Selection management via HandleManager
    void SelectHandle(int index);
    void DeselectHandles();
    int GetSelectedHandleIndex() const;
    const std::vector<Handle>& GetHandles() const { return handleManager.GetHandles(); }

    // Handle manipulation
    void MoveHandle(int index, sf::Vector2f newPos);

    // Train management
    void AddTrain(Train* train);
    void RemoveTrain(Train* train);
    bool HasTrains() const { return !trains.empty(); }
    const std::vector<Train*>& GetTrains() const { return trains; }

    // Property setters and getters
    void SetThickness(float newThickness) { thickness = newThickness; }
    void SetSelected(bool value) { selected = value; }
    const std::string GetName() const { return name; }
    sf::Color GetColor() const { return color; }
    float GetThickness() const { return thickness; }
    bool IsSelected() const { return selected; }

private:
    // Member variables
    std::vector<LinePoint> points;      // Points defining the line (cities and nodes)
    HandleManager handleManager;        // Manages handles for manipulation
    std::vector<Train*> trains;         // Trains traveling on the line
    std::string name;                   // Name of the line
    sf::Color color;                    // Color of the line
    float thickness;                    // Thickness of the line when drawn
    bool selected;                      // Selection state

    // New member variables for offset management
    std::vector<OffsetInfo> offsetInfos;

    // Utility function to normalize a vector
    sf::Vector2f Normalize(const sf::Vector2f& vec) const;
    sf::Vector2f GetPerpendicularVector(int segmentIndex) const;
};
