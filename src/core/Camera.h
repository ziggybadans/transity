#pragma once

#include <SFML/Graphics.hpp>
#include <optional>

namespace sf {
    class Event;
}

class Camera {
public:
    Camera();
    const sf::View& getView() const;
    sf::Vector2f getCenter() const;
    sf::View& getViewToModify();
    void setInitialView(const sf::RenderWindow& window, const sf::Vector2f& landCenter, const sf::Vector2f& landSize);
    void moveView(const sf::Vector2f& offset);
    void zoomView(float factor);

    void onWindowResize(unsigned int width, unsigned int height);

private:
    sf::View _view;
};