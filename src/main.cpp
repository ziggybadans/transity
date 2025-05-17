#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML window");
    sf::View gameView({0, 0}, {800, 600});
    gameView.setCenter({400, 300});
    gameView.setSize({800, 600});

    float cameraSpeed = 200.0f;
    float zoomFactor = 0.9f;
    float unzoomFactor = 1.0f / zoomFactor;
    sf::Clock deltaClock;

    sf::RectangleShape land;
    land.setSize({100, 100});
    land.setFillColor(sf::Color::White);
    land.setPosition({50, 50});

    while (window.isOpen()) {
        sf::Time dt = deltaClock.restart();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            
            if (const auto* scrolledEvent = event->getIf<sf::Event::MouseWheelScrolled>()) {
                sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPosBeforeZoom = window.mapPixelToCoords(mousePixelPos, gameView);
                if (scrolledEvent->wheel == sf::Mouse::Wheel::Vertical) {
                    if (scrolledEvent->delta > 0) {
                        gameView.zoom(zoomFactor);
                    } else if (scrolledEvent->delta < 0) {
                        gameView.zoom(unzoomFactor);
                    }
                    sf::Vector2f worldPosAfterZoom = window.mapPixelToCoords(mousePixelPos, gameView);

                    sf::Vector2f offset = worldPosBeforeZoom - worldPosAfterZoom;
                    gameView.move(offset);
                }
            }
        }

        sf::Vector2f movement(0, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            movement.y -= cameraSpeed * dt.asSeconds();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            movement.y += cameraSpeed * dt.asSeconds();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            movement.x -= cameraSpeed * dt.asSeconds();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            movement.x += cameraSpeed * dt.asSeconds();
        if (movement.x != 0 || movement.y != 0) {
            gameView.move(movement);
        }

        window.setView(gameView);

        sf::Color oceanColor(173, 216, 230);
        window.clear(oceanColor);

        window.draw(land);

        window.display();
    }

    return 0;
}