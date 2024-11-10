#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <limits>
#include "Train.h"
#include "Station.h"

struct LineNode {
    Station* station; // Pointer to station if this node is a station, nullptr otherwise
    sf::Vector2f position; // Position of the node (used if station == nullptr)

    sf::Vector2f GetPosition() const {
        return station ? station->GetPosition() : position;
    }

    bool IsStation() const {
        return station != nullptr;
    }
};

class Line {
public:
    Line();

    // Delete copy constructor and copy assignment operator to prevent copying
    Line(const Line&) = delete;
    Line& operator=(const Line&) = delete;

    // Modify AddNode to accept Station pointer or position
    void AddNode(Station* station);
    void AddNode(const sf::Vector2f& position);

    // Render the line, optionally highlighting it if selected.
    void Render(sf::RenderWindow& window, float zoomLevel, bool isSelected = false) const;

    // Set the active state of the line.
    void SetActive(bool active);
    bool IsActive() const;

    // Get the list of nodes
    const std::vector<LineNode>& GetNodes() const { return nodes; }

    // Methods to set and get the color of the line.
    void SetColor(const sf::Color& color);
    sf::Color GetColor() const;

    // Methods to set and get the thickness of the line.
    void SetThickness(float thickness);
    float GetThickness() const;

    // Provide access to the calculated spline points for smoother rendering.
    const std::vector<sf::Vector2f>& GetSplinePoints() const { return splinePoints; }

    // Methods for train simulation.
    void AddTrain();
    const std::vector<Train>& GetTrains() const;        // Const version
    std::vector<Train>& GetTrains();                    // Non-const version
    void RemoveTrains();

    // Get the total length of the line.
    float GetLength() const;

    // Get the position along the line based on progress (0.0 to 1.0).
    sf::Vector2f GetPositionAlongLine(float progress) const;

    // Get the progress closest to a station along the line, given the current progress.
    float GetClosestStationProgress(float progress) const;

    // Get the progress values of the stations along the line
    const std::vector<float>& GetStationProgressValues() const;

    // Methods to set and get the speed of the line.
    void SetSpeed(float speedKmPerHour);
    float GetSpeed() const;

    // Add getter and setter for editing state.
    void SetEditing(bool editing) { isEditing = editing; }
    bool IsEditing() const { return isEditing; }

    // Method to get the index of a node at a given position
    int GetNodeIndexAtPosition(const sf::Vector2f& position, float zoomLevel) const;

    // Method to set the position of a node (only for non-station nodes)
    void SetNodePosition(int index, const sf::Vector2f& newPosition);

    // Method to regenerate the spline points (make it public)
    void GenerateSplinePoints();

private:
    std::vector<LineNode> nodes; // Nodes representing the points of the line.

    bool active; // Flag indicating whether the line is active.

    // Points calculated along the spline for rendering.
    std::vector<sf::Vector2f> splinePoints;

    // Line properties.
    sf::Color color; // Color of the line.
    float thickness; // Thickness of the line.

    // Trains running on the line.
    std::vector<Train> trains;

    // Total length of the line.
    float totalLength;

    // Store progress values of stations along the line
    std::vector<float> stationProgressValues;

    // Method to calculate station progress values
    void CalculateStationProgressValues();

    float speedInKmPerHour; // Speed of trains on this line in km/h

    bool isEditing = false; // Flag to indicate if the line is in edit mode.
};
