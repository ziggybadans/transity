// Train.cpp
#include "Train.h"
#include "../world/Line.h" // Ensure Line.h is included
#include "../Debug.h" // Assuming Debug.h provides debugging utilities
#include <cmath>

// Constructor
Train::Train(Line* route, const std::string& id, const std::vector<sf::Vector2f>& pathPoints, const std::vector<sf::Vector2f>& stationPositions, float maxSpeed)
    : m_route(route),
    m_id(id),
    m_maxSpeed(maxSpeed),
    m_currentSpeed(0.0f),
    m_selected(false),
    m_forward(true),
    m_state(State::Moving),
    m_waitTime(0.0f),
    m_pathPoints(pathPoints),
    m_stationPositions(stationPositions),
    m_currentPointIndex(1)
{
    if (!m_pathPoints.empty())
    {
        m_position = m_pathPoints[0];
    }
    else
    {
        m_position = sf::Vector2f(0.f, 0.f);
        DEBUG_ERROR("Train constructed with empty path points.");
    }
}

// Destructor
Train::~Train()
{
    // Remove train from the line's train list
    if (m_route) {
        m_route->RemoveTrain(this);
    }
}

// Update train position
void Train::Update(float dt)
{
    switch (m_state)
    {
    case State::Moving:
        Move(dt);
        break;
    case State::Waiting:
        Wait(dt);
        break;
    }
}

void Train::Move(float dt)
{
    if (m_pathPoints.empty() ||
        m_currentPointIndex < 0 ||
        m_currentPointIndex >= static_cast<int>(m_pathPoints.size()))
    {
        return;
    }

    sf::Vector2f targetPos = m_pathPoints[m_currentPointIndex];
    sf::Vector2f toTarget = targetPos - m_position;
    float distanceToTarget = Distance(m_position, targetPos);
    bool nextIsCity = IsCityIndex(m_currentPointIndex);

    if (nextIsCity) {
        float stopDistance = (m_currentSpeed * m_currentSpeed) / (2.0f * DECELERATION);
        if (stopDistance >= distanceToTarget) {
            m_currentSpeed -= DECELERATION * dt;
            if (m_currentSpeed < 0.0f)
                m_currentSpeed = 0.0f;
        }
        else {
            if (m_currentSpeed < m_maxSpeed) {
                m_currentSpeed += ACCELERATION * dt;
                if (m_currentSpeed > m_maxSpeed)
                    m_currentSpeed = m_maxSpeed;
            }
        }
    }
    else {
        if (m_currentSpeed < m_maxSpeed) {
            m_currentSpeed += ACCELERATION * dt;
            if (m_currentSpeed > m_maxSpeed)
                m_currentSpeed = m_maxSpeed;
        }
    }

    sf::Vector2f direction = Normalize(toTarget);
    sf::Vector2f movement = direction * m_currentSpeed * dt;
    float movementDistance = Length(movement);

    if (movementDistance >= distanceToTarget) {
        m_position = targetPos;
        if (nextIsCity) {
            ArriveAtCity();
        }
        else {
            m_currentPointIndex = AdvanceIndex(m_forward);
            m_position = targetPos;
        }
    }
    else {
        m_position += movement;
    }
}

float Train::Length(const sf::Vector2f& v) const
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

void Train::ArriveAtCity()
{
    m_state = State::Waiting;
    m_currentSpeed = 0.0f;
    int stationIndex = -1;
    for (int i = 0; i < static_cast<int>(m_stationPositions.size()); i++) {
        if (Distance(m_position, m_stationPositions[i]) <= PROXIMITY_THRESHOLD) {
            stationIndex = i;
            break;
        }
    }
    if (stationIndex == 0 || stationIndex == static_cast<int>(m_stationPositions.size()) - 1) {
        m_waitTime = STOP_DURATION * 2.0f;
    }
    else {
        m_waitTime = STOP_DURATION;
    }
    if (m_currentPointIndex >= 0 && m_currentPointIndex < static_cast<int>(m_pathPoints.size()))
    {
        m_position = m_pathPoints[m_currentPointIndex];
    }
}

void Train::Wait(float dt)
{
    m_waitTime -= dt;
    if (m_waitTime <= 0.0f)
    {
        m_state = State::Moving;
        if (m_forward)
        {
            m_currentPointIndex++;
            if (m_currentPointIndex >= static_cast<int>(m_pathPoints.size()))
            {
                m_forward = false;
                m_currentPointIndex = static_cast<int>(m_pathPoints.size()) - 2;
            }
        }
        else
        {
            m_currentPointIndex--;
            if (m_currentPointIndex < 0)
            {
                m_forward = true;
                m_currentPointIndex = 1;
            }
        }
    }
}

// Getters
std::string Train::GetID() const
{
    return m_id;
}

sf::Vector2f Train::GetPosition() const
{
    return m_position;
}

float Train::GetSpeed() const
{
    return m_currentSpeed;
}

float Train::GetMaxSpeed() const
{
    return m_maxSpeed;
}

Line* Train::GetRoute() const
{
    return m_route;
}

std::string Train::GetState() const
{
    return (m_state == State::Moving) ? "Moving" : "Waiting";
}

std::string Train::GetDirection() const
{
    return m_forward ? "Forward" : "Reverse";
}

int Train::GetCurrentPointIndex() const
{
    return m_currentPointIndex;
}

float Train::GetWaitTime() const
{
    return m_waitTime;
}

void Train::Draw(sf::RenderWindow& window) const
{
    // For visualization, draw a simple circle or rectangle at the train's position
    sf::CircleShape shape(5.0f); // radius 5
    shape.setFillColor(m_selected ? sf::Color::Red : sf::Color::Green);
    shape.setPosition(m_position - sf::Vector2f(5.f, 5.f)); // Center the circle
    window.draw(shape);
}

// Helper methods
float Train::Distance(const sf::Vector2f& a, const sf::Vector2f& b) const
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

sf::Vector2f Train::Normalize(const sf::Vector2f& v) const
{
    float length = std::sqrt(v.x * v.x + v.y * v.y);
    if (length != 0)
        return v / length;
    else
        return sf::Vector2f(0.f, 0.f);
}

int Train::AdvanceIndex(bool forward)
{
    int newIndex = m_currentPointIndex;
    if (forward) {
        newIndex++;
        if (newIndex >= static_cast<int>(m_pathPoints.size())) {
            m_forward = false;
            newIndex = static_cast<int>(m_pathPoints.size()) - 2;
        }
    }
    else {
        newIndex--;
        if (newIndex < 0) {
            m_forward = true;
            newIndex = 1;
        }
    }
    return newIndex;
}

bool Train::IsCityIndex(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_pathPoints.size()))
        return false;

    sf::Vector2f pointPos = m_pathPoints[index];
    for (const auto& stationPos : m_stationPositions) {
        if (Distance(pointPos, stationPos) <= PROXIMITY_THRESHOLD) {
            return true;
        }
    }
    return false;
}