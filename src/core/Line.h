#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <limits>
#include "Train.h"

class Line {
public:
    Line();

    // Add a node to the line.
    void AddNode(const sf::Vector2f& position);

    // Render the line, optionally highlighting it if selected.
    void Render(sf::RenderWindow& window, float zoomLevel, bool isSelected = false) const;

    // Set the active state of the line.
    void SetActive(bool active);
    bool IsActive() const;

    // Get the list of nodes (positions) that form the line.
    const std::vector<sf::Vector2f>& GetNodes() const { return nodes; }

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

private:
    std::vector<sf::Vector2f> nodes; // Nodes representing the points of the line.
    bool active; // Flag indicating whether the line is active.

    // Helper function to generate spline points for smooth curves.
    void GenerateSplinePoints();

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
};
