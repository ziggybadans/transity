#include "Camera.h"
#include "../Logger.h"
#include <SFML/Window/Event.hpp>

Camera::Camera() {
    LOG_INFO("Camera", "Camera created. Initial view size: (%.1f, %.1f), center: (%.1f, %.1f)", _view.getSize().x, _view.getSize().y, _view.getCenter().x, _view.getCenter().y);
    _view.setSize({800, 600});
    _view.setCenter({400, 300});
}

void Camera::setInitialView(const sf::RenderWindow& window, const sf::Vector2f& landCenter, const sf::Vector2f& landSize) {
    LOG_INFO("Camera", "Setting initial view. Land center: (%.1f, %.1f), Land size: (%.1f, %.1f)", landCenter.x, landCenter.y, landSize.x, landSize.y);
    _view.setCenter(landCenter);

    sf::Vector2u windowSizeU = window.getSize();
    sf::Vector2f windowSizeF(static_cast<float>(windowSizeU.x), static_cast<float>(windowSizeU.y));
    LOG_DEBUG("Camera", "Window size: (%.1f, %.1f)", windowSizeF.x, windowSizeF.y);

    if (windowSizeF.y == 0.0f) {
        LOG_ERROR("Camera", "Window height is zero, cannot calculate aspect ratio. Using default view settings.");
        _view.setSize(sf::Vector2f(800.f, 600.f)); // Default size
        _view.setCenter(landCenter); // Still center on land if possible
        return;
    }

    float windowAspectRatio = windowSizeF.x / windowSizeF.y;

    if (landSize.y == 0.0f) {
        LOG_WARN("Camera", "Land height is zero, cannot calculate land aspect ratio. View may not be correctly framed.");
        // Continue with calculation, windowAspectRatio will be used, but land framing might be off.
    }

    float landAspectRatio = landSize.x / landSize.y;
    LOG_DEBUG("Camera", "Window aspect ratio: %.2f, Land aspect ratio: %.2f", windowAspectRatio, landAspectRatio);

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
    LOG_INFO("Camera", "Initial view set. View size: (%.1f, %.1f), View center: (%.1f, %.1f)", _view.getSize().x, _view.getSize().y, _view.getCenter().x, _view.getCenter().y);
}

sf::View& Camera::getViewToModify() {
    return _view;
}

void Camera::moveView(const sf::Vector2f& offset) {
    _view.move(offset);
    LOG_TRACE("Camera", "View moved by (%.1f, %.1f). New center: (%.1f, %.1f)", offset.x, offset.y, _view.getCenter().x, _view.getCenter().y);
}

void Camera::zoomView(float factor) {
    _view.zoom(factor);
    LOG_TRACE("Camera", "View zoomed by factor %.2f. New size: (%.1f, %.1f)", factor, _view.getSize().x, _view.getSize().y);
}

const sf::View& Camera::getView() const {
    LOG_TRACE("Camera", "Getting view. Center: (%.1f, %.1f), Size: (%.1f, %.1f)", _view.getCenter().x, _view.getCenter().y, _view.getSize().x, _view.getSize().y);
    return _view;
}