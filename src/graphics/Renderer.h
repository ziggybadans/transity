#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <mutex>
#include "Camera.h"
#include "../interfaces/IInitializable.h"

class Renderer : public IInitializable {
public:
    Renderer();
    ~Renderer();

    /* Core Renderer Methods */
    bool Init() override;
    bool InitWithWindow(sf::RenderWindow& window);
    void Render(sf::RenderWindow& window, const Camera& camera);
    void Shutdown();

private:
    /* Renderer State */
    bool m_isInitialized;
    std::mutex m_renderMutex;
};
