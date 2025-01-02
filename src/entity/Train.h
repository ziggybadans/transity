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
        m_currentCityIndex(0),
        m_forward(true),
        m_state(State::Moving),
        m_waitTime(0.0f)
    {
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

    std::string GetDirection() const {
        return m_forward ? "Forward" : "Reverse";
    }

    int GetCurrentCityIndex() const { return m_currentCityIndex; }

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
    int m_currentCityIndex;       // Index of the current city
    bool m_forward;               // Direction of travel
    State m_state;                // Current state (Moving or Waiting)
    float m_waitTime;             // Time left to wait at a city

    // Constants
    const float ACCELERATION = 20.0f;   // Pixels per second squared
    const float DECELERATION = 20.0f;   // Pixels per second squared
    const float STOP_DURATION = 2.0f;   // Seconds to wait at each city

    // Helper methods
    void Move(float dt) {
        const auto& cities = m_route->GetCities();
        if (cities.empty()) return;

        // Determine next city index based on direction
        int targetIndex = m_currentCityIndex;
        if (m_forward) {
            if (m_currentCityIndex >= static_cast<int>(cities.size()) - 1) {
                // Reached end, reverse direction
                m_forward = false;
                targetIndex = m_currentCityIndex - 1;
            }
            else {
                targetIndex = m_currentCityIndex + 1;
            }
        }
        else {
            if (m_currentCityIndex <= 0) {
                // Reached start, reverse direction
                m_forward = true;
                targetIndex = m_currentCityIndex + 1;
            }
            else {
                targetIndex = m_currentCityIndex - 1;
            }
        }

        const sf::Vector2f currentPos = m_position;
        const sf::Vector2f targetPos = cities[targetIndex]->position;
        sf::Vector2f direction = targetPos - currentPos;
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance == 0.0f) return; // Already at the target

        // Normalize direction
        direction /= distance;

        // Determine if we need to start decelerating
        float stoppingDistance = (m_currentSpeed * m_currentSpeed) / (2 * DECELERATION);
        if (distance <= stoppingDistance) {
            // Decelerate
            m_currentSpeed -= DECELERATION * dt;
            if (m_currentSpeed < 0.0f) m_currentSpeed = 0.0f;
        }
        else {
            // Accelerate up to max speed
            m_currentSpeed += ACCELERATION * dt;
            if (m_currentSpeed > m_maxSpeed) m_currentSpeed = m_maxSpeed;
        }

        // Move the train
        sf::Vector2f movement = direction * m_currentSpeed * dt;
        if (std::sqrt(movement.x * movement.x + movement.y * movement.y) > distance) {
            // Reached or overshot the target
            m_position = targetPos;
            m_currentCityIndex = targetIndex;
            m_state = State::Waiting;
            m_waitTime = STOP_DURATION;
            m_currentSpeed = 0.0f;
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
};
