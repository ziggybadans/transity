#include "Camera.h"
#include <SFML/Window/Event.hpp>

Camera::Camera() {
    view.setSize({800, 600});
    view.setCenter({400, 300});
}

void Camera::setInitialView(const sf::RenderWindow& window) {
    sf::Vector2u windowSize = window.getSize();
    view.setSize(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y));
    view.setCenter(static_cast<float>(windowSize.x) / 2.f, static_cast<float>(windowSize.y) / 2.f);
}

void Camera::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    m_inputHandler.handleEvent(event, window, this->view);
}

void Camera::update(sf::Time dt) {
    m_inputHandler.update(dt, this->view);
}

const sf::View& Camera::getView() const {
    return view;
}