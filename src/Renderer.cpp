// src/Renderer.cpp
#include "Renderer.h"
#include "Utilities.h"
#include <cmath>
#include <imgui.h>
#include <imgui-SFML.h>

Renderer::Renderer(sf::RenderWindow& win, sf::View& viewRef, const ChunkManager& cm, int chunkSize, int tileSize, const sf::Vector2f& defaultSize)
    : window(win), view(viewRef), chunkManager(cm), m_chunkSize(chunkSize), m_tileSize(tileSize), defaultViewSize(defaultSize)
{

}

void Renderer::renderFrame() {
    // Clear the window with a black color
    window.clear(sf::Color::Black);
    // Set the view for rendering
    window.setView(view);

    // Draw all relevant chunks
    drawChunks();

    ImGui::SFML::Render(window);

    // Display the rendered frame
    window.display();
}

void Renderer::drawChunks() {
    // Determine current LOD level
    int currentLOD = determineLODLevel();

    // Get a snapshot of loaded chunks
    auto loadedChunks = chunkManager.getLoadedChunks();

    // Iterate over all loaded chunks
    for (const auto& [coord, chunk] : loadedChunks) {
        // Select the appropriate LOD vertex array
        const sf::VertexArray* verticesToDraw;
        switch (currentLOD) {
        case 0:
            verticesToDraw = &chunk.verticesLOD0;
            break;
        case 1:
            verticesToDraw = &chunk.verticesLOD1;
            break;
        case 2:
            verticesToDraw = &chunk.verticesLOD2;
            break;
        default:
            verticesToDraw = &chunk.verticesLOD0;
            break;
        }

        // Draw the chunk without any additional transform
        window.draw(*verticesToDraw);
    }
}

int Renderer::determineLODLevel() {
    float currentScaleX = view.getSize().x / defaultViewSize.x;

    if (currentScaleX != m_lastZoom) {
        m_lastZoom = currentScaleX;
        if (currentScaleX < LOD1_THRESHOLD) {
            m_cachedLOD = 0;
        }
        else if (currentScaleX < LOD2_THRESHOLD) {
            m_cachedLOD = 1;
        }
        else {
            m_cachedLOD = 2;
        }
    }

    return m_cachedLOD;
}