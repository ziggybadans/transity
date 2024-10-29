#pragma once

#include <SFML/Graphics.hpp>

class Line;

class Train {
public:
    Train(Line* line);

    void Update(float deltaTime);
    void Render(sf::RenderWindow& window, float zoomLevel) const;

private:
    Line* line;
    float speed;          // Units per second
    float progress;       // Position along the line [0.0, 1.0]
    bool forward;         // Direction of travel
    mutable sf::CircleShape shape; // Graphical representation of the train

    float waitTime;       // Time to wait at stations
    float currentWaitTime;
    bool isStopped;

    // Helper to get position on the line
    sf::Vector2f getPositionAlongLine() const;
};
