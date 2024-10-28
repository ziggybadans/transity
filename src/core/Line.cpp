// Line.cpp
#include "Line.h"
#include <cmath>

Line::Line()
    : active(true)
{}

void Line::AddNode(const sf::Vector2f& position) {
    nodes.push_back(position);
    GenerateSplinePoints();
}

void Line::GenerateSplinePoints() {
    splinePoints.clear();

    if (nodes.size() < 2) {
        // Not enough points to generate a spline
        return;
    }

    const int numPointsPerSegment = 20; // Adjust for smoother curves

    for (size_t i = 0; i < nodes.size() - 1; ++i) {
        // For Catmull-Rom spline, we need four points for each segment
        sf::Vector2f p0 = (i == 0) ? nodes[i] : nodes[i - 1];
        sf::Vector2f p1 = nodes[i];
        sf::Vector2f p2 = nodes[i + 1];
        sf::Vector2f p3 = (i + 2 < nodes.size()) ? nodes[i + 2] : nodes[i + 1];

        for (int j = 0; j <= numPointsPerSegment; ++j) {
            float t = static_cast<float>(j) / numPointsPerSegment;
            sf::Vector2f point = 0.5f * (
                (2.0f * p1) +
                (-p0 + p2) * t +
                (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
                (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t
                );
            splinePoints.push_back(point);
        }
    }
}

void Line::Render(sf::RenderWindow& window, float zoomLevel) const {
    if (splinePoints.empty()) {
        // Not enough points to render
        return;
    }

    sf::VertexArray lineStrip(sf::LineStrip, splinePoints.size());
    for (size_t i = 0; i < splinePoints.size(); ++i) {
        lineStrip[i].position = splinePoints[i];
        lineStrip[i].color = sf::Color::Blue; // Customize color as needed
    }

    window.draw(lineStrip);
}

void Line::SetActive(bool active) {
    this->active = active;
}

bool Line::IsActive() const {
    return active;
}
