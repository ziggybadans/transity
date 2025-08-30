#include "Camera.h"
#include "Logger.h"
#include <SFML/Window/Event.hpp>

Camera::Camera() {
    LOG_DEBUG("Camera", "Camera created. Initial view size: (%.1f, %.1f), center: (%.1f, %.1f)",
             _view.getSize().x, _view.getSize().y, _view.getCenter().x, _view.getCenter().y);
    _view.setSize({800, 600});
    _view.setCenter({400, 300});
}

void Camera::setInitialView(const sf::RenderWindow &window, const sf::Vector2f &landCenter,
                            const sf::Vector2f &landSize) {
   LOG_DEBUG("Camera", "Setting initial view. Land center: (%.1f, %.1f), Land size: (%.1f, %.1f)",
            landCenter.x, landCenter.y, landSize.x, landSize.y);
   _view.setCenter(landCenter);

    sf::Vector2u windowSizeU = window.getSize();
    sf::Vector2f windowSizeF(static_cast<float>(windowSizeU.x), static_cast<float>(windowSizeU.y));
    LOG_DEBUG("Camera", "Window size: (%.1f, %.1f)", windowSizeF.x, windowSizeF.y);

    if (windowSizeF.y == 0.0f) {
        LOG_ERROR(
            "Camera",
            "Window height is zero, cannot calculate aspect ratio. Using default view settings.");
        _view.setSize(sf::Vector2f(800.f, 600.f));
        _view.setCenter(landCenter);
        return;
    }

    float windowAspectRatio = windowSizeF.x / windowSizeF.y;

    if (landSize.y == 0.0f) {
        LOG_WARN("Camera", "Land height is zero, cannot calculate land aspect ratio. View may not "
                           "be correctly framed.");
    }

    float landAspectRatio = landSize.x / landSize.y;
    LOG_DEBUG("Camera", "Window aspect ratio: %.2f, Land aspect ratio: %.2f", windowAspectRatio,
              landAspectRatio);

    sf::Vector2f viewSize;
    float paddingFactor = 1.1f;

    if (windowAspectRatio > landAspectRatio) {
        viewSize.y = landSize.y * paddingFactor;
        viewSize.x = viewSize.y * windowAspectRatio;
    } else {
        viewSize.x = landSize.x * paddingFactor;
        viewSize.y = viewSize.x / windowAspectRatio;
    }
    _view.setSize(viewSize);
    LOG_DEBUG("Camera", "Initial view set. View size: (%.1f, %.1f), View center: (%.1f, %.1f)",
             _view.getSize().x, _view.getSize().y, _view.getCenter().x, _view.getCenter().y);
}

sf::View &Camera::getViewToModify() noexcept {
    return _view;
}

void Camera::moveView(const sf::Vector2f &offset) noexcept {
    _view.move(offset);
    LOG_TRACE("Camera", "View moved by (%.1f, %.1f). New center: (%.1f, %.1f)", offset.x, offset.y,
              _view.getCenter().x, _view.getCenter().y);
}

void Camera::zoomView(float factor) noexcept {
    _view.zoom(factor);
    LOG_TRACE("Camera", "View zoomed by factor %.2f. New size: (%.1f, %.1f)", factor,
              _view.getSize().x, _view.getSize().y);
}

const sf::View &Camera::getView() const noexcept {
    return _view;
}

sf::Vector2f Camera::getCenter() const noexcept {
    return _view.getCenter();
}

void Camera::onWindowResize(unsigned int width, unsigned int height) noexcept {
    LOG_DEBUG("Camera", "Window resized to %u x %u", width, height);

    _view.setViewport({{0.f, 0.f}, {1.f, 1.f}});

    sf::Vector2f viewSize = _view.getSize();

    if (height == 0) {
        LOG_ERROR("Camera", "Window height is zero, cannot calculate aspect ratio for resize.");
        return;
    }

    float windowAspectRatio = static_cast<float>(width) / static_cast<float>(height);

    viewSize.x = viewSize.y * windowAspectRatio;
    _view.setSize(viewSize);

    LOG_DEBUG("Camera", "View size adjusted for new aspect ratio. New size: (%.1f, %.1f)",
              _view.getSize().x, _view.getSize().y);
}

float Camera::getZoom() const noexcept {

    return 600.0f / _view.getSize().y;
}