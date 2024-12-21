#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <stdexcept>

class Camera {
public:
    Camera(const sf::Vector2u& windowSize);
    ~Camera();

    /* Core Camera Methods */
    void Update(float deltaTime);
    void ApplyView(sf::RenderWindow& window) const;
    void OnResize(const sf::Vector2u& newSize);

    /* Movement Methods */
    void Move(const sf::Vector2f& offset);
    void Zoom(float factor);

    /* Setters */
    void SetPosition(const sf::Vector2f& position);
    void SetZoom(float zoomLevel);
    void SetMinZoomLevel(float value);
    void SetMaxZoomLevel(float value);

    /* Getters */
    sf::Vector2f GetPosition() const;
    float GetZoomLevel() const;
    float GetMinZoomLevel() const { return m_minZoomLevel; }
    float GetMaxZoomLevel() const { return m_maxZoomLevel; }
    const sf::View& GetView() const;

private:

    /* View Properties */
    sf::View m_view;
    sf::Vector2f m_currentPosition;
    float m_currentZoom;

    /* Window Properties */
    sf::Vector2u m_windowSize;
    sf::Vector2f m_baseViewSize;

    /* Zoom Constraints */
    float m_minZoomLevel;
    float m_maxZoomLevel;
};
