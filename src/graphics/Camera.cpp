#include "Camera.h"
#include <iostream>

Camera::Camera(const sf::Vector2u& windowSize)
    : m_currentPosition(0.0f, 0.0f)
    , m_currentZoom(1.0f)
    , m_windowSize(windowSize)
    , m_minZoomLevel(0.001f)
    , m_maxZoomLevel(1.0f)
{
    m_baseViewSize = sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y));
    m_view.setSize(m_baseViewSize);
    m_view.setCenter(m_currentPosition);
}

Camera::~Camera() {}

void Camera::Update(float /*deltaTime*/) {
    m_view.setCenter(m_currentPosition);
}

void Camera::ApplyView(sf::RenderWindow& window) const {
    window.setView(m_view);
}

void Camera::OnResize(const sf::Vector2u& newSize) {
    m_windowSize = newSize;
    m_baseViewSize = sf::Vector2f(static_cast<float>(newSize.x), static_cast<float>(newSize.y));
    m_view.setSize(m_baseViewSize.x * m_currentZoom, m_baseViewSize.y * m_currentZoom);
}

void Camera::Move(const sf::Vector2f& offset) {
    m_currentPosition += offset;
    m_view.setCenter(m_currentPosition);
}

/**
 * @brief Adjusts the zoom level by the given factor
 * @param factor The factor by which to adjust the current zoom level
 * @throws std::invalid_argument if zoom factor is not positive
 */
void Camera::Zoom(float factor) {
    if (factor <= 0.0f) {
        throw std::invalid_argument("Zoom factor must be positive.");
    }
    m_currentZoom = std::clamp(m_currentZoom * factor, m_minZoomLevel, m_maxZoomLevel);
    m_view.setSize(m_baseViewSize.x * m_currentZoom, m_baseViewSize.y * m_currentZoom);
    std::cout << "Camera zoom updated. New Zoom Level: " << m_currentZoom << std::endl;
}

void Camera::SetPosition(const sf::Vector2f& position) {
    m_currentPosition = position;
    m_view.setCenter(m_currentPosition);
    std::cout << "Camera Position: (" << m_currentPosition.x << ", " << m_currentPosition.y << ")" << std::endl;
}

/**
 * @brief Sets the zoom level, clamping between min and max values
 * @param zoomLevel The new zoom level to set
 * @throws std::invalid_argument if zoom level is not positive
 */
void Camera::SetZoom(float zoomLevel) {
    if (zoomLevel <= 0.0f) {
        throw std::invalid_argument("Zoom level must be positive.");
    }
    m_currentZoom = std::clamp(zoomLevel, m_minZoomLevel, m_maxZoomLevel);
    m_view.setSize(m_baseViewSize.x * m_currentZoom, m_baseViewSize.y * m_currentZoom);
}

void Camera::SetMinZoomLevel(float value) {
    m_minZoomLevel = value;
}

void Camera::SetMaxZoomLevel(float value) {
    m_maxZoomLevel = value;
}

sf::Vector2f Camera::GetPosition() const {
    return m_currentPosition;
}

float Camera::GetZoomLevel() const {
    return m_currentZoom;
}

const sf::View& Camera::GetView() const {
    return m_view;
}