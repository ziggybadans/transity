// Train.h
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

// Forward declaration of Line class
class Line;

// A simple Train class that travels along a Line's cities sequentially.
class Train {
public:
    // Constructor
    Train(Line* route, const std::string& id, const std::vector<sf::Vector2f>& pathPoints, const std::vector<sf::Vector2f>& stationPositions, float maxSpeed = 50.0f);

    // Destructor
    ~Train();

    // Update train position
    void Update(float dt);

    // Getters
    std::string GetID() const;
    sf::Vector2f GetPosition() const;
    float GetSpeed() const;
    float GetMaxSpeed() const;
    Line* GetRoute() const;

    // Additional getters for detailed information
    std::string GetState() const;
    std::string GetDirection() const;
    int GetCurrentPointIndex() const;
    float GetWaitTime() const;

    // Setters
    void SetSelected(bool value) { m_selected = value; }

    // Method to draw the train (optional, for visualization)
    void Draw(sf::RenderWindow& window) const;

private:
    // Enumeration for train states
    enum class State {
        Moving,
        Waiting
    };

    // Member variables
    std::string m_id;               // Unique ID for the train
    Line* m_route;                  // Pointer to the line this train travels on
    float m_maxSpeed;               // Maximum speed (pixels per second)
    float m_currentSpeed;           // Current speed
    sf::Vector2f m_position;        // Current position in world space
    bool m_selected;

    // State management
    bool m_forward;                 // Direction of travel
    State m_state;                  // Current state (Moving or Waiting)
    float m_waitTime;               // Time left to wait at a city

    // Path information
    std::vector<sf::Vector2f> m_pathPoints; // Points along the path
    int m_currentPointIndex;                // Current path point index
    std::vector<sf::Vector2f> m_stationPositions;

    // Constants
    const float ACCELERATION = 20.0f;        // Pixels per second squared
    const float DECELERATION = 20.0f;        // Pixels per second squared
    const float STOP_DURATION = 2.0f;        // Seconds to wait at each city
    const float PROXIMITY_THRESHOLD = 5.0f;  // Distance to consider as arrived at the city

    // Helper methods
    void Move(float dt);
    void Wait(float dt);
    void ArriveAtCity();
    bool IsCityIndex(int index) const;
    int AdvanceIndex(bool forward);

    // Utility functions
    float Distance(const sf::Vector2f& a, const sf::Vector2f& b) const;
    float Length(const sf::Vector2f& v) const;
    sf::Vector2f Normalize(const sf::Vector2f& v) const;
};
