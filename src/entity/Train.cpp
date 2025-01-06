// Train.cpp
#include "Train.h"
#include "../world/Line.h" // Include the Line class definition
#include "../Debug.h"
#include <algorithm>
#include <cmath>

// Constructor
Train::Train(Line* route, const std::string& id, float maxSpeed)
    : m_route(route),
    m_maxSpeed(maxSpeed),
    m_currentSpeed(0.0f),
    m_position(route->GetCities().front()->GetPosition()),
    m_currentPointIndex(0),
    m_forward(true),
    m_state(State::Waiting),
    m_waitTime(STOP_DURATION),
    m_id(id)
{
    m_pathPoints = m_route->GetPathPoints();

    // If there's a valid path, start at the very first point
    if (!m_pathPoints.empty()) {
        m_position = m_pathPoints.front();
    }

    // Initialize city indices
    m_cityIndices = m_route->GetCityIndices();
}

// Destructor
Train::~Train() {}

// Update train position
void Train::Update(float dt) {
    switch (m_state) {
    case State::Moving:
        Move(dt);
        break;
    case State::Waiting:
        Wait(dt);
        break;
    }
}

// Getters
std::string Train::GetID() const {
    return m_id;
}

sf::Vector2f Train::GetPosition() const {
    return m_position;
}

float Train::GetSpeed() const {
    return m_currentSpeed;
}

float Train::GetMaxSpeed() const {
    return m_maxSpeed;
}

Line* Train::GetRoute() const {
    return m_route;
}

// Additional getters
std::string Train::GetState() const {
    switch (m_state) {
    case State::Moving:
        return "Moving";
    case State::Waiting:
        return "Waiting";
    default:
        return "Unknown";
    }
}

std::string Train::GetDirection() const {
    return m_forward ? "Forward" : "Reverse";
}

int Train::GetCurrentPointIndex() const {
    return m_currentPointIndex;
}

float Train::GetWaitTime() const {
    return m_waitTime;
}

// Draw the train
void Train::Draw(sf::RenderWindow& window) const {
    sf::CircleShape shape(6.0f); // Simple representation
    shape.setOrigin(6.0f, 6.0f); // Center the circle
    shape.setPosition(m_position);
    // Selection color is handled by Renderer via SelectionManager
    shape.setFillColor(sf::Color::Red);
    window.draw(shape);
}

// Helper Methods

void Train::Move(float dt) {
    if (m_pathPoints.empty()) return;

    // 1. Determine the next path point index based on direction
    int targetIndex = m_currentPointIndex + (m_forward ? 1 : -1);

    // 2. Check for direction reversal at the ends
    if (targetIndex >= static_cast<int>(m_pathPoints.size())) {
        // Reached the end going forward
        m_forward = false;
        targetIndex = m_currentPointIndex - 1;

        // Arrive at the end city
        if (IsCityIndex(m_currentPointIndex)) {
            ArriveAtCity();
            return; // Exit to wait before moving
        }
    }
    else if (targetIndex < 0) {
        // Reached the front going backward
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
        // Check if it's a city
        if (IsCityIndex(m_currentPointIndex)) {
            ArriveAtCity();
        }
        return;
    }

    // 3. Normalize direction to the next path point
    sf::Vector2f direction = toNextPt / distToNextPt;

    // 4. Find the next city to perform speed logic
    int cityIndex = FindNextCityIndex();
    if (cityIndex < 0) {
        // No further cities in this direction
        if (m_currentSpeed < m_maxSpeed) {
            m_currentSpeed = std::min(m_maxSpeed, m_currentSpeed + ACCELERATION * dt);
        }
    }
    else {
        // Next city is at m_pathPoints[cityIndex]
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

    // 5. Move the train toward the next path point using the current speed
    sf::Vector2f movement = direction * m_currentSpeed * dt;

    // If movement overshoots the next path point, clamp to it
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

void Train::Wait(float dt) {
    m_waitTime -= dt;
    if (m_waitTime <= 0.0f) {
        m_state = State::Moving;
    }
}

void Train::ArriveAtCity() {
    m_state = State::Waiting;
    m_waitTime = STOP_DURATION;
    m_currentSpeed = 0.0f;
}

int Train::FindNextCityIndex() const {
    if (m_pathPoints.empty()) return -1;

    if (m_forward) {
        // Find the first city index greater than currentPointIndex
        for (int cityIdx : m_cityIndices) {
            if (cityIdx > m_currentPointIndex) {
                return cityIdx;
            }
        }
    }
    else {
        // Find the last city index less than currentPointIndex
        for (auto it = m_cityIndices.rbegin(); it != m_cityIndices.rend(); ++it) {
            if (*it < m_currentPointIndex) {
                return *it;
            }
        }
    }
    return -1; // No city found in that direction
}

bool Train::IsCityIndex(int index) const {
    return std::find(m_cityIndices.begin(), m_cityIndices.end(), index) != m_cityIndices.end();
}

// Utility functions
float Train::Distance(const sf::Vector2f& a, const sf::Vector2f& b) const {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

float Train::Length(const sf::Vector2f& v) const {
    return std::sqrt(v.x * v.x + v.y * v.y);
}
