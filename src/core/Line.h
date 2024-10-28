// Line.h
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Line {
public:
    Line();

    // Add a node to the line
    void AddNode(const sf::Vector2f& position);

    // Render the line
    void Render(sf::RenderWindow& window, float zoomLevel) const;

    void SetActive(bool active);
    bool IsActive() const;

    const std::vector<sf::Vector2f>& GetNodes() const { return nodes; }

private:
    std::vector<sf::Vector2f> nodes;
    bool active;

    // Helper function to generate spline points
    void GenerateSplinePoints();

    // Points calculated along the spline for rendering
    std::vector<sf::Vector2f> splinePoints;
};
