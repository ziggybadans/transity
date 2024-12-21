#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <mutex>
#include "Camera.h"
#include "../interfaces/IInitializable.h"
#include "../world/Map.h"

class Renderer : public IInitializable {
public:
    Renderer();
    ~Renderer();

    /* Core Renderer Methods */
    bool Init() override;
    bool InitWithWindow(sf::RenderWindow& window);
    void Render(sf::RenderWindow& window, const Camera& camera, const Map& map);
    void Shutdown();

private:
    /* Renderer State */
    bool m_isInitialized;
    std::mutex m_renderMutex;

    /* World Rendering */
    void RenderMap(sf::RenderWindow& window, const Map& map);
};
