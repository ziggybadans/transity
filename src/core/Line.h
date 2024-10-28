// Line.h
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Line {
public:
    Line();

    void AddNode(const sf::Vector2f& position);

    void Render(sf::RenderWindow& window, float zoomLevel) const;

    void SetActive(bool active);

    bool IsActive() const;

    const std::vector<sf::Vector2f>& GetNodes() const { return nodes; }

private:
    std::vector<sf::Vector2f> nodes;
    sf::VertexArray lineVertices;
    bool active;

    void UpdateLineVertices(float zoomLevel);
};