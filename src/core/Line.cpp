// Line.cpp
#include "Line.h"

Line::Line()
    : active(true), lineVertices(sf::LineStrip)
{}

void Line::AddNode(const sf::Vector2f& position, bool curved) {
    nodes.push_back(position);
    curves.push_back(curved);
    UpdateLineVertices(1.0f); // Initial zoomLevel, will be updated in Render
}

void Line::UpdateLineVertices(float zoomLevel) {
    lineVertices.clear();
    for (const auto& node : nodes) {
        // Scale node position based on zoomLevel if necessary
        // Assuming nodes are stored in world coordinates, no need to scale
        lineVertices.append(sf::Vertex(node, sf::Color::Blue));
    }
}

void Line::Render(sf::RenderWindow& window, float zoomLevel) const {
    // Optionally, adjust line thickness based on zoomLevel
    // SFML's VertexArray does not support line thickness directly
    // Alternative: Use sf::RectangleShape or shaders for thicker lines

    // For simplicity, render as is
    window.draw(lineVertices);
}

void Line::SetActive(bool active) {
    this->active = active;
}

bool Line::IsActive() const {
    return active;
}
