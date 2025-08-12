#pragma once

#include <SFML/Graphics.hpp>
#include <optional>

namespace sf {
class Event;
}

class Camera {
public:
    Camera();
    const sf::View &getView() const noexcept;
    sf::Vector2f getCenter() const noexcept;
    sf::View &getViewToModify() noexcept;
    void setInitialView(const sf::RenderWindow &window, const sf::Vector2f &landCenter,
                        const sf::Vector2f &landSize);
    void moveView(const sf::Vector2f &offset) noexcept;
    void zoomView(float factor) noexcept;
    float getZoom() const noexcept;

    void onWindowResize(unsigned int width, unsigned int height) noexcept;

private:
    sf::View _view;
};