#pragma once

#include <SFML/Graphics.hpp>

class Line;

class Train {
public:
    // Constructor that initializes the train on a specific line.
    Train(Line* line);

    // Updates the train's position and state based on the elapsed time.
    void Update(float deltaTime);

    // Renders the train to the provided window, adjusting for zoom level.
    void Render(sf::RenderWindow& window, float zoomLevel) const;

private:
    Line* line; // Pointer to the line the train is currently on.
    float speed; // Speed of the train in units per second.
    float progress; // Position along the line, ranging from 0.0 to 1.0.
    bool forward; // Direction of travel, true if moving forward.
    mutable sf::CircleShape shape; // Graphical representation of the train.

    float waitTime; // Total time the train waits at each station.
    float currentWaitTime; // Current wait time elapsed at a station.
    bool isStopped; // Indicates if the train is currently stopped at a station.

    // Timer for logging every second
    float logTimer; // Accumulates elapsed time for logging

    // Helper function to get the position of the train along the line.
    sf::Vector2f getPositionAlongLine() const;
};
