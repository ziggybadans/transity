// src/Renderer.cpp
#include "Renderer.h"

#include <cmath>
#include <imgui.h>
#include <imgui-SFML.h>
#include <stdexcept>

Renderer::Renderer(sf::RenderWindow& win, sf::View& viewRef, ChunkManager& cm,
    int chunkSize, int tileSize, const sf::Vector2f& defaultSize)
    : m_window(win), m_view(viewRef), m_chunkManager(cm), m_chunkSize(chunkSize),
    m_tileSize(tileSize), m_defaultViewSize(defaultSize),
    m_vaLODs() // Default initialization
{
    // Validate inputs
    if (m_chunkSize <= 0) {
        throw std::invalid_argument("chunkSize must be positive.");
    }
    if (m_tileSize <= 0) {
        throw std::invalid_argument("tileSize must be positive.");
    }

    // Initialize each VertexArray with the appropriate PrimitiveType
    for (auto& va : m_vaLODs) {
        va.setPrimitiveType(sf::Quads); // Assuming all LODs use Quads
    }
}

void Renderer::renderFrame() {
    // Clear the window with a black color
    m_window.clear(sf::Color::Black);

    // Set the view for rendering
    m_window.setView(m_view);

    // Get current view bounds for frustum culling
    sf::FloatRect viewBounds(m_view.getCenter() - m_view.getSize() / 2.0f, m_view.getSize());

    // Load all chunks that are visible within the current view bounds
    loadVisibleChunks(viewBounds);

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
        // Iterate through thresholds to determine LOD
        m_cachedLOD = 0; // Default to highest detail
        for (size_t i = 0; i < NUM_LODS - 1; ++i) {
            if (currentScaleX >= LOD_THRESHOLDS[i] && currentScaleX < LOD_THRESHOLDS[i + 1]) {
                m_cachedLOD = static_cast<int>(i + 1);
                break;
            }
        }
        if (currentScaleX >= LOD_THRESHOLDS[NUM_LODS - 1]) {
            m_cachedLOD = static_cast<int>(NUM_LODS - 1);
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
        if (chunkPtr && isChunkVisible(coord, viewBounds)) {
            m_visibleChunks.emplace(coord, chunkPtr);
        }
    }
}

void Renderer::updateVertexArrays(int currentLOD) {
    for (auto& va : m_vaLODs) {
        va.clear();
    }

    for (const auto& [coord, chunkPtr] : m_visibleChunks) {
        if (!chunkPtr) continue;

        const sf::VertexArray* verticesToDraw = nullptr;
        switch (currentLOD) {
        case 0: verticesToDraw = &chunkPtr->verticesLOD0; break;
        case 1: verticesToDraw = &chunkPtr->verticesLOD1; break;
        case 2: verticesToDraw = &chunkPtr->verticesLOD2; break;
        case 3: verticesToDraw = &chunkPtr->verticesLOD3; break;
        case 4: verticesToDraw = &chunkPtr->verticesLOD4; break;
        default: verticesToDraw = &chunkPtr->verticesLOD0; break;
        }

        if (verticesToDraw && verticesToDraw->getVertexCount() > 0) {
            // Append each vertex individually
            for (size_t i = 0; i < verticesToDraw->getVertexCount(); ++i) {
                m_vaLODs[currentLOD].append((*verticesToDraw)[i]);
            }

            // Optionally, reset the update flag
            switch (currentLOD) {
            case 0: chunkPtr->needsUpdateLOD0 = false; break;
            case 1: chunkPtr->needsUpdateLOD1 = false; break;
            case 2: chunkPtr->needsUpdateLOD2 = false; break;
            case 3: chunkPtr->needsUpdateLOD3 = false; break;
            case 4: chunkPtr->needsUpdateLOD4 = false; break;
            }
        }
    }
}

void Renderer::drawChunks(int currentLOD) {
    // Draw VertexArrays based on the current LOD
    if (currentLOD >= 0 && static_cast<size_t>(currentLOD) < m_vaLODs.size()) {
        if (m_vaLODs[currentLOD].getVertexCount() > 0) {
            m_window.draw(m_vaLODs[currentLOD]);
        }
    }
}

void Renderer::loadVisibleChunks(const sf::FloatRect& viewBounds) {
    float chunkPixelSize = static_cast<float>(m_chunkSize * m_tileSize);

    // Calculate the range of chunks that intersect with the view bounds
    int firstChunkX = static_cast<int>(std::floor(viewBounds.left / chunkPixelSize));
    int lastChunkX = static_cast<int>(std::floor((viewBounds.left + viewBounds.width) / chunkPixelSize));
    int firstChunkY = static_cast<int>(std::floor(viewBounds.top / chunkPixelSize));
    int lastChunkY = static_cast<int>(std::floor((viewBounds.top + viewBounds.height) / chunkPixelSize));

    // Clamp chunk indices to valid range based on your world size
    // Assuming ChunkManager knows the world dimensions
    int maxChunkX = m_chunkManager.getWorldChunksX() - 1;
    int maxChunkY = m_chunkManager.getWorldChunksY() - 1;

    firstChunkX = std::max(firstChunkX, 0);
    firstChunkY = std::max(firstChunkY, 0);
    lastChunkX = std::min(lastChunkX, maxChunkX);
    lastChunkY = std::min(lastChunkY, maxChunkY);

    // Iterate through the range and load chunks if not already loaded
    for (int y = firstChunkY; y <= lastChunkY; ++y) {
        for (int x = firstChunkX; x <= lastChunkX; ++x) {
            ChunkCoord coord{ x, y };
            if (!m_chunkManager.isChunkLoaded(x, y)) {
                // Generate and load the chunk
                std::shared_ptr<Chunk> newChunk = m_chunkManager.generateChunk(x, y);
                m_chunkManager.addLoadedChunk(x, y, newChunk);
            }
        }
    }

    // Optionally, unload chunks that are no longer visible to save memory
    // This step is optional and depends on your game's requirements
    auto loadedChunks = m_chunkManager.getLoadedChunks();
    for (const auto& [coord, chunkPtr] : loadedChunks) {
        float chunkLeft = coord.x * chunkPixelSize;
        float chunkTop = coord.y * chunkPixelSize;
        sf::FloatRect chunkRect(chunkLeft, chunkTop, chunkPixelSize, chunkPixelSize);
        if (!viewBounds.intersects(chunkRect)) {
            m_chunkManager.unloadChunk(coord.x, coord.y);
        }
    }
}
