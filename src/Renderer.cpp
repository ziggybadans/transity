// src/Renderer.cpp
#include "Renderer.h"

#include <cmath>
#include <imgui.h>
#include <imgui-SFML.h>
#include <stdexcept>

Renderer::Renderer(sf::RenderWindow& win, sf::View& viewRef, const ChunkManager& cm,
    int chunkSize, int tileSize, const sf::Vector2f& defaultSize)
    : m_window(win), m_view(viewRef), m_chunkManager(cm), m_chunkSize(chunkSize),
    m_tileSize(tileSize), m_defaultViewSize(defaultSize),
    m_vaLOD0(sf::Quads), m_vaLOD1(sf::Quads), m_vaLOD2(sf::Quads)
{
    // Validate inputs
    if (m_chunkSize <= 0) {
        throw std::invalid_argument("chunkSize must be positive.");
    }
    if (m_tileSize <= 0) {
        throw std::invalid_argument("tileSize must be positive.");
    }

    // Initialize VertexArrays
    m_vaLOD0.clear();
    m_vaLOD1.clear();
    m_vaLOD2.clear();
}

void Renderer::renderFrame() {
    // Clear the window with a black color
    m_window.clear(sf::Color::Black);

    // Set the view for rendering
    m_window.setView(m_view);

    // Get current view bounds for frustum culling
    sf::FloatRect viewBounds(m_view.getCenter() - m_view.getSize() / 2.0f, m_view.getSize());

    // Update visible chunks based on the current view
    updateVisibleChunks(viewBounds);

    // Determine current LOD level
    int currentLOD = determineLODLevel();

    // Update VertexArrays with new visible chunks data
    updateVertexArrays(currentLOD);

    // Draw all relevant chunks
    drawChunks(currentLOD);

    // Render ImGui elements
    ImGui::SFML::Render(m_window);

    // Display the rendered frame
    m_window.display();
}

int Renderer::determineLODLevel() {
    float currentScaleX = m_view.getSize().x / m_defaultViewSize.x;

    if (std::abs(currentScaleX - m_lastZoom) > 1e-5f) { // Use epsilon for float comparison
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

bool Renderer::isChunkVisible(const ChunkCoord& coord, const sf::FloatRect& viewBounds) const {
    float chunkPixelSize = static_cast<float>(m_chunkSize * m_tileSize);
    sf::FloatRect chunkRect(coord.x * chunkPixelSize, coord.y * chunkPixelSize,
        chunkPixelSize, chunkPixelSize);
    return viewBounds.intersects(chunkRect);
}

void Renderer::updateVisibleChunks(const sf::FloatRect& viewBounds) {
    // Get a snapshot of loaded chunks
    auto loadedChunks = m_chunkManager.getLoadedChunks();

    // Clear current visibleChunks
    m_visibleChunks.clear();

    // Iterate over all loaded chunks and add to visibleChunks if visible
    for (const auto& [coord, chunkPtr] : loadedChunks) {
        if (isChunkVisible(coord, viewBounds)) {
            m_visibleChunks.emplace(coord, chunkPtr);
        }
    }
}

void Renderer::updateVertexArrays(int currentLOD) {
    // Clear existing VertexArrays
    m_vaLOD0.clear();
    m_vaLOD1.clear();
    m_vaLOD2.clear();

    // Iterate over all visible chunks
    for (const auto& [coord, chunkPtr] : m_visibleChunks) {
        if (!chunkPtr) continue; // Safety check

        const sf::VertexArray* verticesToDraw = nullptr;
        switch (currentLOD) {
        case 0:
            verticesToDraw = &chunkPtr->verticesLOD0;
            break;
        case 1:
            verticesToDraw = &chunkPtr->verticesLOD1;
            break;
        case 2:
            verticesToDraw = &chunkPtr->verticesLOD2;
            break;
        default:
            verticesToDraw = &chunkPtr->verticesLOD0;
            break;
        }

        if (verticesToDraw->getVertexCount() > 0) {
            // Append all vertices to the corresponding VertexArray
            sf::VertexArray& targetVA = (currentLOD == 0) ? m_vaLOD0 :
                (currentLOD == 1) ? m_vaLOD1 : m_vaLOD2;

            // Iterate using an index-based loop
            for (std::size_t i = 0; i < verticesToDraw->getVertexCount(); ++i) {
                targetVA.append((*verticesToDraw)[i]);
            }
        }
    }
}

void Renderer::drawChunks(int currentLOD) {
    // Draw VertexArrays based on the current LOD
    switch (currentLOD) {
    case 0:
        if (m_vaLOD0.getVertexCount() > 0) {
            m_window.draw(m_vaLOD0);
        }
        break;
    case 1:
        if (m_vaLOD1.getVertexCount() > 0) {
            m_window.draw(m_vaLOD1);
        }
        break;
    case 2:
        if (m_vaLOD2.getVertexCount() > 0) {
            m_window.draw(m_vaLOD2);
        }
        break;
    default:
        if (m_vaLOD0.getVertexCount() > 0) {
            m_window.draw(m_vaLOD0);
        }
        break;
    }
}
