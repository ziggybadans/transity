#include "Renderer.h"
#include <iostream>

Renderer::Renderer() 
    : m_isInitialized(false)
{
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Init(sf::RenderWindow& window) {
    if (m_isInitialized) return true;

    m_isInitialized = true;
    std::cout << "Renderer initialized successfully." << std::endl;
    return true;
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!m_isInitialized) return;

    std::lock_guard<std::mutex> lock(m_renderMutex);
    camera.ApplyView(window);
}

void Renderer::Shutdown() {
    m_isInitialized = false;
}