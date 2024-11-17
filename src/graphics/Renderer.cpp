#include "Renderer.h"
#include <iostream>
#include "../Debug.h"

Renderer::Renderer() 
    : m_isInitialized(false)
{
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Init() {
    // Basic initialization that doesn't require a window
    m_isInitialized = false;
    return true;
}

bool Renderer::InitWithWindow(sf::RenderWindow& window) {
    if (m_isInitialized) return true;

    m_isInitialized = true;
    DEBUG_INFO("Renderer initialized successfully.");
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