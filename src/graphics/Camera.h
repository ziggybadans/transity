#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <stdexcept>

class Camera {
public:
    // Constructor to initialize the camera with the window size.
    Camera(const sf::Vector2u& windowSize);
    ~Camera();

    // Setters for immediate position and absolute zoom level.
    void SetPosition(const sf::Vector2f& position);
    void SetZoom(float zoomLevel);

    // Getters for the current position and zoom level.
    sf::Vector2f GetPosition() const;
    float GetZoomLevel() const;

    // Updates the camera's position and constraints every frame.
    void Update(float deltaTime);

    // Applies the view to the provided window.
    void ApplyView(sf::RenderWindow& window) const;

    // Zoom function to adjust zoom level by a factor.
    void Zoom(float factor);

    // Adjusts the camera view when the window is resized.
    void OnResize(const sf::Vector2u& newSize);

    // Sets the boundaries of the world to prevent the camera from going beyond the world limits.
    void SetWorldBounds(float width, float height);

    // Set minimum and maximum zoom levels to prevent over or under zooming.
    void setMinZoomLevel(float value);
    void setMaxZoomLevel(float value);

    // Getters for minimum and maximum zoom levels.
    float getMinZoomLevel() const { return minZoomLevel; }
    float getMaxZoomLevel() const { return maxZoomLevel; }

    // Returns a reference to the current view.
    const sf::View& GetView() const;

private:
    sf::View view; // The SFML view representing the camera.

    // Current camera states.
    sf::Vector2f currentPosition; // The current position of the camera.
    float currentZoom; // The current zoom level of the camera.

    sf::Vector2u windowSize; // The size of the window.
    sf::Vector2f baseViewSize; // The original size of the view.

    // World boundaries to ensure the camera stays within defined limits.
    float worldWidth; // Width of the world.
    float worldHeight; // Height of the world.

    // Minimum and maximum zoom levels to constrain zooming.
    float minZoomLevel; // Minimum zoom level.
    float maxZoomLevel; // Maximum zoom level.

    // Helper function to clamp the camera's position within world boundaries.
    void ClampPosition();
};
