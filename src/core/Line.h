#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <limits>
#include "Train.h"

class Line {
public:
    Line();

    // Add a node to the line
    void AddNode(const sf::Vector2f& position);

    // Render the line
    void Render(sf::RenderWindow& window, float zoomLevel, bool isSelected = false) const;

    void SetActive(bool active);
    bool IsActive() const;

    const std::vector<sf::Vector2f>& GetNodes() const { return nodes; }

    // Methods for color and thickness
    void SetColor(const sf::Color& color);
    sf::Color GetColor() const;

    void SetThickness(float thickness);
    float GetThickness() const;

    // Provide access to spline points
    const std::vector<sf::Vector2f>& GetSplinePoints() const { return splinePoints; }

    // Methods for train simulation
    void AddTrain();
    const std::vector<Train>& GetTrains() const;        // Const version
    std::vector<Train>& GetTrains();                    // Non-const version

    float GetLength() const;
    sf::Vector2f GetPositionAlongLine(float progress) const;
    float GetClosestStationProgress(float progress) const;

private:
    std::vector<sf::Vector2f> nodes;
    bool active;

    // Helper function to generate spline points
    void GenerateSplinePoints();

    // Points calculated along the spline for rendering
    std::vector<sf::Vector2f> splinePoints;

    // New members
    sf::Color color;
    float thickness;

    // New members for train simulation
    std::vector<Train> trains;

    // Length of the line
    float totalLength;
};
