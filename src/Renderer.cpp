// src/Renderer.cpp
#include "Renderer.h"
#include "Utilities.h"
#include <cmath>

Renderer::Renderer(sf::RenderWindow& win, sf::View& viewRef, const ChunkManager& cm, int chunkSize, int tileSize)
    : window(win), view(viewRef), chunkManager(cm), CHUNK_SIZE(chunkSize), TILE_SIZE(tileSize)
{
}

void Renderer::renderFrame() {
    // Clear the window with a black color
    window.clear(sf::Color::Black);
    // Set the view for rendering
    window.setView(view);

    // Draw all relevant chunks
    drawChunks();

    // Display the rendered frame
    window.display();
}

void Renderer::drawChunks() {
    // Determine which chunks need to be rendered based on the view
    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();

    // Calculate world size in pixels
    float worldSizeX = chunkManager.WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE;
    float worldSizeY = chunkManager.WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE;

    // Calculate the bounds of the view
    float leftBound = center.x - size.x / 2;
    float topBound = center.y - size.y / 2;

    // Calculate the range of visible chunks
    int firstChunkX = static_cast<int>(std::floor(leftBound / (CHUNK_SIZE * TILE_SIZE)));
    int firstChunkY = static_cast<int>(std::floor(topBound / (CHUNK_SIZE * TILE_SIZE)));

    // Number of chunks visible horizontally and vertically
    int visibleChunksX = static_cast<int>(std::ceil(size.x / (CHUNK_SIZE * TILE_SIZE))) + 2; // +2 for buffer
    int visibleChunksY = static_cast<int>(std::ceil(size.y / (CHUNK_SIZE * TILE_SIZE))) + 2; // +2 for buffer

    // Iterate over the 3x3 grid
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            // Calculate the base chunk indices
            int baseChunkX = firstChunkX + dx * chunkManager.WORLD_CHUNKS_X;
            int baseChunkY = firstChunkY + dy * chunkManager.WORLD_CHUNKS_Y;

            // Iterate through all visible chunks
            for (int y = 0; y < visibleChunksY; ++y) {
                for (int x = 0; x < visibleChunksX; ++x) {
                    int chunkX = baseChunkX + x;
                    int chunkY = baseChunkY + y;

                    // Wrap the chunk indices
                    int wrappedChunkX = Utilities::wrapCoordinate(chunkX, chunkManager.WORLD_CHUNKS_X);
                    int wrappedChunkY = Utilities::wrapCoordinate(chunkY, chunkManager.WORLD_CHUNKS_Y);

                    // Access the chunk
                    const Chunk& currentChunk = chunkManager.getChunk(wrappedChunkX, wrappedChunkY);

                    // Calculate position shift based on grid offset
                    float shiftX = dx * worldSizeX;
                    float shiftY = dy * worldSizeY;

                    // Create a transform for the shifted position
                    sf::Transform transform;
                    transform.translate(shiftX, shiftY);

                    // Draw the chunk with the applied transform
                    window.draw(currentChunk.vertices, transform);

                    // Draw the contour lines
                    //window.draw(currentChunk.contourLines, transform);
                }
            }
        }
    }
}
