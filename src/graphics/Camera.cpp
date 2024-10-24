#include "Camera.h"

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

Camera::~Camera() {}

void Camera::SetWorldBounds(float width, float height) {
    worldWidth = width;
    worldHeight = height;
    ClampPosition();
}

void Camera::setMinZoomLevel(float value) {
    minZoomLevel = value;
}

void Camera::setMaxZoomLevel(float value) {
    maxZoomLevel = value;
}

void Camera::SetPosition(const sf::Vector2f& position) {
    currentPosition = position;
    ClampPosition();
    view.setCenter(currentPosition);
}

void Camera::SetZoom(float zoomLevel) {
    if (zoomLevel <= 0.0f) {
        throw std::invalid_argument("Zoom level must be positive.");
    }
    currentZoom = std::clamp(zoomLevel, minZoomLevel, maxZoomLevel);
    view.setSize(baseViewSize.x * currentZoom, baseViewSize.y * currentZoom);
    ClampPosition();
}

sf::Vector2f Camera::GetPosition() const {
    return currentPosition;
}

float Camera::GetZoomLevel() const {
    return currentZoom;
}

void Camera::Update(float /*deltaTime*/) {
    view.setCenter(currentPosition);
    ClampPosition();
}

void Camera::ApplyView(sf::RenderWindow& window) const {
    window.setView(view);
}

void Camera::Zoom(float factor) {
    if (factor <= 0.0f) {
        throw std::invalid_argument("Zoom factor must be positive.");
    }
    currentZoom = std::clamp(currentZoom * factor, minZoomLevel, maxZoomLevel);
    view.setSize(baseViewSize.x * currentZoom, baseViewSize.y * currentZoom);
    ClampPosition();
}

void Camera::OnResize(const sf::Vector2u& newSize) {
    windowSize = newSize;
    baseViewSize = sf::Vector2f(static_cast<float>(newSize.x), static_cast<float>(newSize.y));
    view.setSize(baseViewSize.x * currentZoom, baseViewSize.y * currentZoom);
    ClampPosition();
}

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

const sf::View& Camera::GetView() const {
    return view;
}
