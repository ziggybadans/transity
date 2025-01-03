#pragma once

#include <SFML/Graphics.hpp>
#include "../world/Map.h"

// A simple Train class that travels along a Line's cities sequentially.
class Train {
public:
    // Constructor
    // route: which line the train will travel on
    // maxSpeed: maximum speed in pixels-per-second
    Train(Line* route, float maxSpeed = 50.0f)
        : m_route(route),
        m_maxSpeed(maxSpeed),
        m_currentSpeed(0.0f),
        m_position(route->GetCities().front()->position),
        m_selected(false),
        m_currentPointIndex(0),
        m_forward(true),
        m_state(State::Waiting),
        m_waitTime(STOP_DURATION)
    {
        // Generate the entire path (with intermediate Bezier points).
        // `m_cityIndices` will mark which indices correspond to actual cities.
        m_pathPoints = m_route->GetPathPoints(100, m_cityIndices);

        // If there's a valid path, start at the very first point
        if (!m_pathPoints.empty()) {
            m_position = m_pathPoints.front();
        }
    }

    // Update train position
    // dt: delta time from your game loop
    void Update(float dt) {
        switch (m_state) {
        case State::Moving:
            Move(dt);
            break;
        case State::Waiting:
            Wait(dt);
            break;
        }
    }

    // Simple getters
    sf::Vector2f GetPosition() const { return m_position; }
    float GetSpeed() const { return m_currentSpeed; }
    float GetMaxSpeed() const { return m_maxSpeed; }
    bool IsSelected() const { return m_selected; }
    void SetSelected(bool value) { m_selected = value; }
    Line* GetRoute() const { return m_route; }

    // Additional getters for detailed information
    std::string GetState() const {
        switch (m_state) {
        case State::Moving:
            return "Moving";
        case State::Waiting:
            return "Waiting";
        default:
            return "Unknown";
        }
    }

    std::string GetDirection() const { return m_forward ? "Forward" : "Reverse"; }
    int GetCurrentPointIndex() const { return m_currentPointIndex; }
    float GetWaitTime() const { return m_waitTime; }

    // Method to draw the train (optional, for visualization)
    void Draw(sf::RenderWindow& window) const {
        sf::CircleShape shape(5.0f); // Simple representation
        shape.setFillColor(m_selected ? sf::Color::Red : sf::Color::Green);
        shape.setPosition(m_position);
        shape.setOrigin(5.0f, 5.0f); // Center the circle
        window.draw(shape);
    }

private:
    enum class State {
        Moving,
        Waiting
    };

    Line* m_route;                // Pointer to the line this train travels on
    float m_maxSpeed;             // Maximum speed (pixels per second)
    float m_currentSpeed;         // Current speed
    sf::Vector2f m_position;      // Current position in world space
    bool m_selected;

    // State management
    bool m_forward;               // Direction of travel
    State m_state;                // Current state (Moving or Waiting)
    float m_waitTime;             // Time left to wait at a city

    // Path
    std::vector<sf::Vector2f> m_pathPoints;
    std::vector<int> m_cityIndices; // Indices in m_pathPoints that correspond to cities
    int m_currentPointIndex;        // Index of the current point
    int m_nextCityIndex;            // Index of the next city in the path

    // Constants
    const float ACCELERATION = 20.0f;   // Pixels per second squared
    const float DECELERATION = 20.0f;   // Pixels per second squared
    const float STOP_DURATION = 2.0f;   // Seconds to wait at each city
    const float PROXIMITY_THRESHOLD = 5.0f; // Distance to consider as arrived at the city

    // Helper methods
    void Move(float dt) {
        if (m_pathPoints.empty()) return;

        // 1. Figure out the next path point index for stepping along the curve.
        int targetIndex = m_currentPointIndex + (m_forward ? 1 : -1);

        // 2. Check if we need to reverse direction (end of line) or clamp index.
        if (targetIndex >= static_cast<int>(m_pathPoints.size())) {
            // reached the end going forward
            m_forward = false;
            targetIndex = m_currentPointIndex - 1;

            // Arrive at the end city
            if (IsCityIndex(m_currentPointIndex)) {
                ArriveAtCity();
                return; // Exit to wait before moving
            }
        }
        else if (targetIndex < 0) {
            // reached the front going backward
            m_forward = true;
            targetIndex = m_currentPointIndex + 1;

            // Arrive at the start city
            if (IsCityIndex(m_currentPointIndex)) {
                ArriveAtCity();
                return; // Exit to wait before moving
            }
        }

        sf::Vector2f nextPoint = m_pathPoints[targetIndex];
        sf::Vector2f toNextPt = nextPoint - m_position;
        float distToNextPt = Length(toNextPt);

        // If we're basically on top of the next point, skip movement & city check
        if (distToNextPt < 0.001f) {
            m_currentPointIndex = targetIndex;
            // Possibly check if it's a city
            if (IsCityIndex(m_currentPointIndex)) {
                ArriveAtCity();
            }
            return;
        }

        // 3. Normalize direction to the next path point for the actual movement
        sf::Vector2f direction = toNextPt / distToNextPt;

        // 4. Find the NEXT city (not the next path point) to do speed logic
        int cityIndex = FindNextCityIndex();
        if (cityIndex < 0) {
            // No further cities in this direction => do normal acceleration or stay at max?
            // Or optionally reverse direction. For now, we just accelerate if not at max.
            if (m_currentSpeed < m_maxSpeed) {
                m_currentSpeed = std::min(m_maxSpeed, m_currentSpeed + ACCELERATION * dt);
            }
        }
        else {
            // Next city is at pathPoints[ cityIndex ]
            sf::Vector2f cityPos = m_pathPoints[cityIndex];
            float distToCity = Distance(m_position, cityPos);
            float stoppingDistance = (m_currentSpeed * m_currentSpeed) / (2.f * DECELERATION);

            // Decelerate if within stopping distance, otherwise accelerate
            if (distToCity <= stoppingDistance) {
                m_currentSpeed -= DECELERATION * dt;
                if (m_currentSpeed < 0.f) m_currentSpeed = 0.f;
            }
            else {
                if (m_currentSpeed < m_maxSpeed) {
                    m_currentSpeed += ACCELERATION * dt;
                    if (m_currentSpeed > m_maxSpeed) m_currentSpeed = m_maxSpeed;
                }
            }
        }

        // 5. Move the train toward the next path point using the final speed
        sf::Vector2f movement = direction * m_currentSpeed * dt;

        // If we would overshoot the next path point, clamp to it
        if (Length(movement) > distToNextPt) {
            m_position = nextPoint;
            m_currentPointIndex = targetIndex;

            // Check if this path point is a city
            if (IsCityIndex(m_currentPointIndex)) {
                ArriveAtCity();
            }
        }
        else {
            m_position += movement;
        }
    }

    void Wait(float dt) {
        m_waitTime -= dt;
        if (m_waitTime <= 0.0f) {
            m_state = State::Moving;
        }
    }

    void ArriveAtCity() {
        m_state = State::Waiting;
        m_waitTime = STOP_DURATION;
        m_currentSpeed = 0.0f;
    }

    // Finds the index of the next city based on current position and direction
    int FindNextCityIndex() const {
        if (m_pathPoints.empty()) return -1;

        if (m_forward) {
            // We look for the first city index that is greater than m_currentPointIndex
            for (int cityIdx : m_cityIndices) {
                if (cityIdx > m_currentPointIndex) {
                    return cityIdx;
                }
            }
        }
        else {
            // We look for the last city index that is less than m_currentPointIndex
            for (auto it = m_cityIndices.rbegin(); it != m_cityIndices.rend(); ++it) {
                if (*it < m_currentPointIndex) {
                    return *it;
                }
            }
        }
        return -1; // no city found in that direction
    }

    bool IsCityIndex(int index) const {
        // Check if 'index' is in m_cityIndices
        return std::find(m_cityIndices.begin(), m_cityIndices.end(), index) != m_cityIndices.end();
    }

    // Utility: distance between two points
    float Distance(const sf::Vector2f& a, const sf::Vector2f& b) const {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    // Utility: length of a vector
    float Length(const sf::Vector2f& v) const {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }
};
