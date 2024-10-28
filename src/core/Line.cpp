#include "Line.h"

Line::Line()
    : active(true), lineVertices(sf::LineStrip)
{}

void Line::AddNode(const sf::Vector2f& position) {
    nodes.push_back(position);
    UpdateLineVertices(1.0f); // Assuming 1.0f is the default zoom level
}

void Line::UpdateLineVertices(float zoomLevel) {
    lineVertices.clear();
    for (const auto& node : nodes) {
        // Scale the node position based on zoomLevel if necessary
        // For example:
        sf::Vector2f scaledPosition = node * zoomLevel;
        lineVertices.append(sf::Vertex(scaledPosition, sf::Color::Blue));
    }
}

void Line::Render(sf::RenderWindow& window, float zoomLevel) const {
    window.draw(lineVertices);
}

void Line::SetActive(bool active) {
    this->active = active;
}

bool Line::IsActive() const {
    return active;
}