
#pragma once

#include "../core/ServiceLocator.h"  
#include "../event/InputEvents.h"
#include "../event/LineEvents.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class InputHandler {
public:
    
    InputHandler(ServiceLocator &serviceLocator);

    
    void handleGameEvent(const sf::Event &event, sf::RenderWindow &window);
    void update(sf::Time dt);

private:
    ServiceLocator &_services;  

    
    float _zoomFactor;
    float _unzoomFactor;
};
