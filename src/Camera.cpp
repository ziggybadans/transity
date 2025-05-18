#include "Camera.h"
#include <SFML/Window/Event.hpp>

Camera::Camera() {
    view.setSize({800, 600});
    view.setCenter({400, 300});
}

void Camera::setInitialView(const sf::RenderWindow& window, const sf::Vector2f& landCenter, const sf::Vector2f& landSize) {
    view.setCenter(landCenter);

    sf::Vector2u windowSizeU = window.getSize();
    sf::Vector2f windowSizeF(static_cast<float>(windowSizeU.x), static_cast<float>(windowSizeU.y));

    float windowAspectRatio = windowSizeF.x / windowSizeF.y;
    float landAspectRatio = landSize.x / landSize.y;

    sf::Vector2f viewSize;
    float paddingFactor = 1.1f; // Adjust this to control how much "padding" around the land

    if (windowAspectRatio > landAspectRatio) {
        viewSize.y = landSize.y * paddingFactor;
        viewSize.x = viewSize.y * windowAspectRatio;
    } else {
        viewSize.x = landSize.x * paddingFactor;
        viewSize.y = viewSize.x / windowAspectRatio;
    }
    view.setSize(viewSize);
}

std::optional<sf::Vector2f> Camera::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    return m_inputHandler.handleEvent(event, window, this->view);
}

void Camera::update(sf::Time dt) {
    m_inputHandler.update(dt, this->view);
}

const sf::View& Camera::getView() const {
    return view;
}