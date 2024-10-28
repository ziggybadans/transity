#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

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

    // New methods for color and thickness
    void SetColor(const sf::Color& color);
    sf::Color GetColor() const;

    void SetThickness(float thickness);
    float GetThickness() const;

    // Provide access to spline points
    const std::vector<sf::Vector2f>& GetSplinePoints() const { return splinePoints; }

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
};
