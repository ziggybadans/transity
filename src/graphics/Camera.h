#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <stdexcept>

class Camera {
public:
    Camera(const sf::Vector2u& windowSize);
    ~Camera();

    // Setters for immediate position and absolute zoom
    void SetPosition(const sf::Vector2f& position);
    void SetZoom(float zoomLevel);

    // Getters for current states
    sf::Vector2f GetPosition() const;
    float GetZoomLevel() const;

    // Update function to maintain constraints
    void Update(float deltaTime);

    // Apply the view to the window
    void ApplyView(sf::RenderWindow& window) const;

    // Immediate zoom (e.g., for input handling)
    void Zoom(float factor);

    // Adjust view when window is resized
    void OnResize(const sf::Vector2u& newSize);

    // Set world boundaries
    void SetWorldBounds(float width, float height);

    // Set minimum and maximum zoom levels
    void setMinZoomLevel(float value);
    void setMaxZoomLevel(float value);

    float getMinZoomLevel() const { return minZoomLevel; }
    float getMaxZoomLevel() const { return maxZoomLevel; }

    const sf::View& GetView() const;

private:
    sf::View view;

    // Current states
    sf::Vector2f currentPosition;
    float currentZoom;

    sf::Vector2u windowSize;
    sf::Vector2f baseViewSize;

    // World boundaries
    float worldWidth;
    float worldHeight;

    float minZoomLevel;
    float maxZoomLevel;

    // Clamping logic
    void ClampPosition();
};
