#pragma once // Use pragma once for modern C++

#include "../input/InputHandler.h"
#include "../core/Camera.h"
#include <SFML/Graphics/RenderWindow.hpp>

class CameraSystem {
public:
    // Constructor now takes dependencies
    CameraSystem(InputHandler& inputHandler, Camera& camera, sf::RenderWindow& window);

    // Update method no longer needs parameters
    void update();

private:
    // Store references to dependencies
    InputHandler& m_inputHandler;
    Camera& m_camera;
    sf::RenderWindow& m_window;
};
