#pragma once

#include <SFML/Graphics.hpp>
#include "../world/Map.h"

// A simple Train class that travels along a Line's cities sequentially.
class Train {
public:
    // Constructor
    // route: which line the train will travel on
    // speed: speed in some units-per-second (pixels-per-second if your positions are pixel coordinates)
    Train(Line* route, float speed = 50.0f)
        : m_route(route)
        , m_speed(speed)
        , m_currentCityIndex(0)
        , m_position(0.f, 0.f)
        , m_selected(false)
    {
        if (m_route && !m_route->GetCities().empty()) {
            // Start at the first city
            m_position = m_route->GetCities().front()->position;
        }
    }

    // Update train position
    // dt: delta time from your game loop
    void Update(float dt) {
        if (!m_route) return;
        const auto& cities = m_route->GetCities();

        // If there's 0 or 1 city in the line, there's nowhere to go
        if (cities.size() < 2) return;

        // Next city index in the route
        m_nextCityIndex = m_currentCityIndex + 1;
        // If we're at the last city, let's loop or just return
        if (m_nextCityIndex >= static_cast<int>(cities.size())) {
            // Example: loop the train back to the first city
            m_nextCityIndex = 0;
        }

        // Current city and next city positions
        sf::Vector2f currentPos = cities[m_currentCityIndex]->position;
        sf::Vector2f targetPos = cities[m_nextCityIndex]->position;

        // Calculate direction and distance to target city
        sf::Vector2f direction = targetPos - currentPos;
        float distanceTotal = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        // If distance is tiny, avoid dividing by zero
        if (distanceTotal < 0.001f) {
            m_currentCityIndex = m_nextCityIndex; // Snap to next city
            return;
        }

        // Normalize direction
        direction /= distanceTotal;

        // Calculate how far the train moves this update
        float distanceToMove = m_speed * dt;

        // If the distanceToMove >= distance to target city, train arrives and possibly moves on
        if (distanceToMove >= distanceTotal) {
            // Snap to target
            m_position = targetPos;
            m_currentCityIndex = m_nextCityIndex;
        }
        else {
            // Move partially along the way
            m_position += direction * distanceToMove;
        }
    }

    // Simple getters
    sf::Vector2f GetPosition() const { return m_position; }
    float GetSpeed() const { return m_speed; }
    void SetSelected(bool value) { m_selected = value; }
    bool IsSelected() const { return m_selected; }
    Line* GetRoute() const { return m_route; }
    int GetCurrentCityIndex() const { return m_currentCityIndex; }
    int GetNextCityIndex() const { return m_nextCityIndex; }

private:
    Line* m_route;            // Pointer to the line this train travels on
    float m_speed;            // Movement speed
    int   m_currentCityIndex; // Which city in the route we're moving from
    int   m_nextCityIndex;
    sf::Vector2f m_position;  // Current position in world space
    bool m_selected;
};
