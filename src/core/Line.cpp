#include "Line.h"
#include <cmath>

Line::Line()
    : active(true), color(sf::Color::Blue), thickness(2.0f), totalLength(0.0f)
{}

void Line::AddNode(const sf::Vector2f& position) {
    nodes.push_back(position);
    GenerateSplinePoints();
}

void Line::GenerateSplinePoints() {
    splinePoints.clear();
    totalLength = 0.0f;

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

            if (!splinePoints.empty()) {
                sf::Vector2f prevPoint = splinePoints.back();
                totalLength += std::hypot(point.x - prevPoint.x, point.y - prevPoint.y);
            }

            splinePoints.push_back(point);
        }
    }
}

void Line::Render(sf::RenderWindow& window, float zoomLevel, bool isSelected) const {
    if (splinePoints.empty()) {
        // Not enough points to render
        return;
    }

    // Create a vertex array to draw the line with thickness
    sf::VertexArray lineStrip(sf::TrianglesStrip);

    // Adjust thickness based on zoom level
    float scaledThickness = thickness * zoomLevel;

    sf::Color renderColor = color;
    if (isSelected) {
        renderColor = sf::Color::Red; // Indicate selection with red color
    }

    // Generate quads along the line to represent thickness
    for (size_t i = 0; i < splinePoints.size() - 1; ++i) {
        sf::Vector2f point1 = splinePoints[i];
        sf::Vector2f point2 = splinePoints[i + 1];

        // Calculate direction vector
        sf::Vector2f direction = point2 - point1;
        float length = std::hypot(direction.x, direction.y);
        if (length == 0.0f) continue; // Avoid division by zero

        // Normalize direction
        direction /= length;

        // Calculate normal vector
        sf::Vector2f normal(-direction.y, direction.x);

        // Calculate vertices for the quad
        sf::Vector2f offset = normal * (scaledThickness / 2.0f);

        sf::Vertex v1(point1 + offset, renderColor);
        sf::Vertex v2(point1 - offset, renderColor);
        sf::Vertex v3(point2 + offset, renderColor);
        sf::Vertex v4(point2 - offset, renderColor);

        // Add vertices to the array
        lineStrip.append(v1);
        lineStrip.append(v2);
        lineStrip.append(v3);
        lineStrip.append(v4);
    }

    window.draw(lineStrip);
}

void Line::SetActive(bool active) {
    this->active = active;
}

bool Line::IsActive() const {
    return active;
}

void Line::SetColor(const sf::Color& color) {
    this->color = color;
}

sf::Color Line::GetColor() const {
    return color;
}

void Line::SetThickness(float thickness) {
    this->thickness = thickness;
}

float Line::GetThickness() const {
    return thickness;
}

void Line::AddTrain() {
    trains.emplace_back(this);
}

const std::vector<Train>& Line::GetTrains() const {
    return trains;
}

std::vector<Train>& Line::GetTrains() {
    return trains;
}

float Line::GetLength() const {
    return totalLength;
}

sf::Vector2f Line::GetPositionAlongLine(float progress) const {
    if (splinePoints.empty()) {
        return sf::Vector2f();
    }

    float targetDistance = totalLength * progress;
    float accumulatedDistance = 0.0f;

    for (size_t i = 0; i < splinePoints.size() - 1; ++i) {
        sf::Vector2f p1 = splinePoints[i];
        sf::Vector2f p2 = splinePoints[i + 1];
        float segmentLength = std::hypot(p2.x - p1.x, p2.y - p1.y);

        if (accumulatedDistance + segmentLength >= targetDistance) {
            float remaining = targetDistance - accumulatedDistance;
            float t = remaining / segmentLength;
            return p1 + t * (p2 - p1);
        }
        accumulatedDistance += segmentLength;
    }
    return splinePoints.back();
}

float Line::GetClosestStationProgress(float progress) const {
    if (nodes.empty()) {
        return progress;
    }

    float targetDistance = totalLength * progress;

    float minDistance = std::numeric_limits<float>::max();
    float closestProgress = progress;

    for (const auto& nodePosition : nodes) {
        // Find distance along the line to this node
        float nodeDistance = 0.0f;
        float accumulatedDistance = 0.0f;
        for (size_t j = 0; j < splinePoints.size() - 1; ++j) {
            sf::Vector2f p1 = splinePoints[j];
            sf::Vector2f p2 = splinePoints[j + 1];
            float segmentLength = std::hypot(p2.x - p1.x, p2.y - p1.y);
            accumulatedDistance += segmentLength;

            if (std::hypot(p2.x - nodePosition.x, p2.y - nodePosition.y) <= 1e-3f) {
                nodeDistance = accumulatedDistance;
                break;
            }
        }

        float distanceDiff = std::abs(targetDistance - nodeDistance);
        if (distanceDiff < minDistance) {
            minDistance = distanceDiff;
            closestProgress = nodeDistance / totalLength;
        }
    }

    return closestProgress;
}
