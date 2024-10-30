#include "Camera.h"
#include <iostream>

/**
<summary>
Camera class handles the in-game camera view, allowing zooming, panning, and clamping to keep the view within the world bounds.
The camera ensures that the visible portion of the world fits within specified limits, providing a consistent user experience.
</summary>
*/
Camera::Camera(const sf::Vector2u& windowSize)
    : currentPosition(0.0f, 0.0f),
    currentZoom(1.0f),
    windowSize(windowSize),
    worldWidth(10000.0f),
    worldHeight(10000.0f),
    minZoomLevel(0.01f),
    maxZoomLevel(5.0f)
{
    baseViewSize = sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y));
    view.setSize(baseViewSize);
    view.setCenter(currentPosition);
}

/**
<summary>
Destructor for the Camera class.
</summary>
*/
Camera::~Camera() {}

/**
<summary>
Sets the bounds of the world that the camera should stay within.
</summary>
<param name="width">The width of the world.</param>
<param name="height">The height of the world.</param>
*/
void Camera::SetWorldBounds(float width, float height) {
    worldWidth = width;
    worldHeight = height;
    ClampPosition();
}

/**
<summary>
Sets the minimum zoom level allowed for the camera.
</summary>
<param name="value">The minimum zoom level value.</param>
*/
void Camera::setMinZoomLevel(float value) {
    minZoomLevel = value;
}

/**
<summary>
Sets the maximum zoom level allowed for the camera.
</summary>
<param name="value">The maximum zoom level value.</param>
*/
void Camera::setMaxZoomLevel(float value) {
    maxZoomLevel = value;
}

/**
<summary>
Sets the position of the camera within the world.
The position is clamped to ensure that it stays within the world bounds.
</summary>
<param name="position">The new position for the camera.</param>
*/
void Camera::SetPosition(const sf::Vector2f& position) {
    currentPosition = position;
    ClampPosition();
    view.setCenter(currentPosition);
}

/**
<summary>
Sets the zoom level of the camera, clamping it between the minimum and maximum zoom levels.
</summary>
<param name="zoomLevel">The new zoom level for the camera.</param>
<exception cref="std::invalid_argument">Thrown if the zoom level is not positive.</exception>
*/
void Camera::SetZoom(float zoomLevel) {
    if (zoomLevel <= 0.0f) {
        throw std::invalid_argument("Zoom level must be positive.");
    }
    currentZoom = std::clamp(zoomLevel, minZoomLevel, maxZoomLevel);
    view.setSize(baseViewSize.x * currentZoom, baseViewSize.y * currentZoom);
    ClampPosition();
}

/**
<summary>
Gets the current position of the camera in the world.
</summary>
<returns>The current position of the camera as a vector.</returns>
*/
sf::Vector2f Camera::GetPosition() const {
    return currentPosition;
}

/**
<summary>
Gets the current zoom level of the camera.
</summary>
<returns>The current zoom level as a float.</returns>
*/
float Camera::GetZoomLevel() const {
    return currentZoom;
}

/**
<summary>
Updates the camera's position. This method ensures that the view stays centered at the current position.
</summary>
<param name="deltaTime">Time elapsed since the last frame.</param>
*/
void Camera::Update(float /*deltaTime*/) {
    view.setCenter(currentPosition);
    ClampPosition();
}

/**
<summary>
Applies the current camera view to the provided render window.
</summary>
<param name="window">Reference to the SFML RenderWindow that the view will be applied to.</param>
*/
void Camera::ApplyView(sf::RenderWindow& window) const {
    window.setView(view);
}

/**
<summary>
Adjusts the zoom level of the camera by the given factor, clamping the result between the minimum and maximum zoom levels.
</summary>
<param name="factor">The factor by which to adjust the current zoom level.</param>
<exception cref="std::invalid_argument">Thrown if the zoom factor is not positive.</exception>
*/
void Camera::Zoom(float factor) {
    if (factor <= 0.0f) {
        throw std::invalid_argument("Zoom factor must be positive.");
    }
    currentZoom = std::clamp(currentZoom * factor, minZoomLevel, maxZoomLevel);
    view.setSize(baseViewSize.x * currentZoom, baseViewSize.y * currentZoom);
    ClampPosition();

    std::cout << "Zoom level is " << currentZoom << std::endl;
}

/**
<summary>
Handles resizing of the render window by adjusting the view size accordingly.
</summary>
<param name="newSize">The new size of the render window.</param>
*/
void Camera::OnResize(const sf::Vector2u& newSize) {
    windowSize = newSize;
    baseViewSize = sf::Vector2f(static_cast<float>(newSize.x), static_cast<float>(newSize.y));
    view.setSize(baseViewSize.x * currentZoom, baseViewSize.y * currentZoom);
    ClampPosition();
}

/**
<summary>
Ensures that the camera's position is clamped within the bounds of the world.
This method prevents the camera from moving beyond the edges of the defined world space.
</summary>
*/
void Camera::ClampPosition() {
    float halfViewWidth = view.getSize().x / 2.0f;
    float halfViewHeight = view.getSize().y / 2.0f;

    if (worldWidth < view.getSize().x) {
        currentPosition.x = worldWidth / 2.0f;
    }
    else {
        currentPosition.x = std::clamp(currentPosition.x, halfViewWidth, worldWidth - halfViewWidth);
    }

    if (worldHeight < view.getSize().y) {
        currentPosition.y = worldHeight / 2.0f;
    }
    else {
        currentPosition.y = std::clamp(currentPosition.y, halfViewHeight, worldHeight - halfViewHeight);
    }

    view.setCenter(currentPosition);
}

/**
<summary>
Gets the current SFML View object that represents the camera's viewport.
</summary>
<returns>Constant reference to the SFML View.</returns>
*/
const sf::View& Camera::GetView() const {
    return view;
}
