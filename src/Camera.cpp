#include "Camera.h"
#include "Logger.h"
#include <SFML/Window/Event.hpp>

Camera::Camera() {
    LOG_INFO("Camera", "Camera created. Initial view size: (%.1f, %.1f), center: (%.1f, %.1f)", view.getSize().x, view.getSize().y, view.getCenter().x, view.getCenter().y);
    view.setSize({800, 600});
    view.setCenter({400, 300});
}

void Camera::setInitialView(const sf::RenderWindow& window, const sf::Vector2f& landCenter, const sf::Vector2f& landSize) {
    LOG_INFO("Camera", "Setting initial view. Land center: (%.1f, %.1f), Land size: (%.1f, %.1f)", landCenter.x, landCenter.y, landSize.x, landSize.y);
    view.setCenter(landCenter);

    sf::Vector2u windowSizeU = window.getSize();
    sf::Vector2f windowSizeF(static_cast<float>(windowSizeU.x), static_cast<float>(windowSizeU.y));
    LOG_DEBUG("Camera", "Window size: (%.1f, %.1f)", windowSizeF.x, windowSizeF.y);

    float windowAspectRatio = windowSizeF.x / windowSizeF.y;
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
    view.setSize(viewSize);
    LOG_INFO("Camera", "Initial view set. View size: (%.1f, %.1f), View center: (%.1f, %.1f)", view.getSize().x, view.getSize().y, view.getCenter().x, view.getCenter().y);
}

std::optional<sf::Vector2f> Camera::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    LOG_TRACE("Camera", "Handling event type %d", event.type);
    return m_inputHandler.handleEvent(event, window, this->view);
}

void Camera::update(sf::Time dt) {
    LOG_TRACE("Camera", "Updating camera state with dt: %f", dt.asSeconds());
    m_inputHandler.update(dt, this->view);
}

const sf::View& Camera::getView() const {
    LOG_TRACE("Camera", "Getting view. Center: (%.1f, %.1f), Size: (%.1f, %.1f)", view.getCenter().x, view.getCenter().y, view.getSize().x, view.getSize().y);
    return view;
}