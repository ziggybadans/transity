#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// Forward declaration of Line class
class Line;
class Passenger;

// A simple Train class that travels along a Line's cities sequentially.
class Train {
public:
    // Constructor
    Train(Line* route, const std::string& id, const std::vector<sf::Vector2f>& pathPoints, const std::vector<sf::Vector2f>& stationPositions, float maxSpeed = 50.0f);

    // Destructor
    ~Train();

    nlohmann::json Serialize() const;
    void Deserialize(const nlohmann::json& j);

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

    void SetCapacity(int capacity) { m_capacity = capacity; }
    int GetCapacity() const { return m_capacity; }
    int GetPassengerCount() const { return static_cast<int>(m_passengers.size()); }
    const std::vector<Passenger*>& GetPassengers() const { return m_passengers; }
    bool HasCapacity() const { return GetPassengerCount() < m_capacity; }
    void AddPassenger(Passenger* p) {
        if (HasCapacity()) m_passengers.push_back(p);
    }
    void RemovePassenger(Passenger* p) {
        auto it = std::remove(m_passengers.begin(), m_passengers.end(), p);
        if (it != m_passengers.end()) m_passengers.erase(it, m_passengers.end());
    }

    const std::vector<sf::Vector2f>& GetPathPoints() const { return m_pathPoints; }
    const std::vector<sf::Vector2f>& GetStationPositions() const { return m_stationPositions; }
    float GetOrientationAngle() const { return std::atan2(m_direction.y, m_direction.x) * 180.0f / 3.14159265f; }

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
    int m_capacity;
    std::vector<Passenger*> m_passengers;
    sf::Vector2f m_direction;

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
