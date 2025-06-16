#ifndef CAMERASYSTEM_H
#define CAMERASYSTEM_H

#include "../input/InputHandler.h"
#include "../core/Camera.h"
#include <SFML/Graphics/RenderWindow.hpp>

class CameraSystem {
public:
    void update(InputHandler& inputHandler, Camera& camera, sf::RenderWindow& window);
};

#endif // CAMERASYSTEM_H