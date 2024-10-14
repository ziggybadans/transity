#include <SFML/Graphics.hpp>

int main() {
    // Create a window with dimensions 800x600 and title "2D Transport Game"
    sf::RenderWindow window(sf::VideoMode(800, 600), "2D Transport Game");

    // Set a frame rate limit
    window.setFramerateLimit(60);

    // Main game loop
    while (window.isOpen()) {
        // Event processing
        sf::Event event;
        while (window.pollEvent(event)) {
            // Close the window if the close button is pressed
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Clear the screen with a black color
        window.clear(sf::Color::Black);

        // Update and draw everything here (future steps)

        // Display the current frame
        window.display();
    }

    return 0;
}
