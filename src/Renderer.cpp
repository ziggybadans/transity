// src/Renderer.cpp
#include "Renderer.h"
#include "Utilities.h"
#include <cmath>
#include <imgui.h>
#include <imgui-SFML.h>

Renderer::Renderer(sf::RenderWindow& win, sf::View& viewRef, const ChunkManager& cm, int chunkSize, int tileSize, const sf::Vector2f& defaultSize)
    : window(win), view(viewRef), chunkManager(cm), m_chunkSize(chunkSize), m_tileSize(tileSize), defaultViewSize(defaultSize)
{
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            float shiftX = dx * (chunkManager.WORLD_CHUNKS_X * m_chunkSize * m_tileSize);
            float shiftY = dy * (chunkManager.WORLD_CHUNKS_Y * m_chunkSize * m_tileSize);
            sf::Transform transform;
            transform.translate(shiftX, shiftY);
            m_gridTransforms.emplace_back(transform);
        }
    }
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

    // Determine which chunks need to be rendered based on the view
    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();

    // Calculate world size in pixels
    float worldSizeX = chunkManager.WORLD_CHUNKS_X * m_chunkSize * m_tileSize;
    float worldSizeY = chunkManager.WORLD_CHUNKS_Y * m_chunkSize * m_tileSize;

    // Calculate the bounds of the view
    float leftBound = center.x - size.x / 2;
    float topBound = center.y - size.y / 2;

    // Calculate the range of visible chunks
    int firstChunkX = static_cast<int>(std::floor(leftBound / (m_chunkSize * m_tileSize)));
    int firstChunkY = static_cast<int>(std::floor(topBound / (m_chunkSize * m_tileSize)));

    // Number of chunks visible horizontally and vertically
    int visibleChunksX = static_cast<int>(std::ceil(size.x / (m_chunkSize * m_tileSize))) + 2; // +2 for buffer
    int visibleChunksY = static_cast<int>(std::ceil(size.y / (m_chunkSize * m_tileSize))) + 2; // +2 for buffer

    // Iterate over the 3x3 grid
    int transformIndex = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            sf::Transform& gridTransform = m_gridTransforms[transformIndex++];

            // Calculate the base chunk indices
            int baseChunkX = firstChunkX + dx * chunkManager.WORLD_CHUNKS_X;
            int baseChunkY = firstChunkY + dy * chunkManager.WORLD_CHUNKS_Y;

            // Iterate through all visible chunks
            for (int y = 0; y < visibleChunksY; ++y) {
                for (int x = 0; x < visibleChunksX; ++x) {
                    int chunkX = baseChunkX + x;
                    int chunkY = baseChunkY + y;

                    // Wrap the x-axis and clamp the y-axis
                    int wrappedChunkX = Utilities::wrapCoordinate(chunkX, chunkManager.WORLD_CHUNKS_X);
                    int wrappedChunkY = chunkY;

                    // Clamp y-axis
                    if (chunkY < 0)
                        wrappedChunkY = 0;
                    if (chunkY >= chunkManager.WORLD_CHUNKS_Y)
                        wrappedChunkY = chunkManager.WORLD_CHUNKS_Y - 1;

                    // Access the chunk
                    const Chunk& currentChunk = chunkManager.getChunk(wrappedChunkX, wrappedChunkY);

                    // Select the appropriate LOD vertex array
                    const sf::VertexArray* verticesToDraw;
                    switch (currentLOD) {
                    case 0:
                        verticesToDraw = &currentChunk.verticesLOD0;
                        break;
                    case 1:
                        verticesToDraw = &currentChunk.verticesLOD1;
                        break;
                    case 2:
                        verticesToDraw = &currentChunk.verticesLOD2;
                        break;
                    default:
                        verticesToDraw = &currentChunk.verticesLOD0;
                        break;
                    }

                    // Draw the chunk with the applied transform
                    window.draw(*verticesToDraw, gridTransform);

                    /* // Optionally, draw contour lines only for high detail
                    if (currentLOD == 0) {
                        window.draw(currentChunk.contourLines, gridTransform);
                    }*/
                }
            }
        }
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