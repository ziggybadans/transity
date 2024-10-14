// src/Game.cpp

#include "Game.h"
#include <cmath>

// Helper function to wrap a float coordinate
float Game::wrapFloat(float coord, float max) {
    float result = fmod(coord, max);
    if (result < 0)
        result += max;
    return result;
}

// Constructor
Game::Game(int chunkSize, int tileSize)
    : CHUNK_SIZE(chunkSize), TILE_SIZE(tileSize) {
    // Calculate world size in pixels
    float worldSizeX = WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE;
    float worldSizeY = WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE;

    // Create a window with a specified resolution
    window.create(sf::VideoMode(800, 600), "2D Transport Game");
    window.setFramerateLimit(120);

    // Set initial view to match the window size
    view = sf::View(sf::FloatRect(0, 0, 800, 600));
    view.setCenter(worldSizeX / 2.0f, worldSizeY / 2.0f);

    // Pre-generate all chunks synchronously
    chunks.resize(WORLD_CHUNKS_X * WORLD_CHUNKS_Y);
    for (int y = 0; y < WORLD_CHUNKS_Y; ++y) {
        for (int x = 0; x < WORLD_CHUNKS_X; ++x) {
            int index = y * WORLD_CHUNKS_X + x;
            chunks[index] = generateChunk(x, y);
        }
    }
}

// Destructor
Game::~Game() {
}

// Access chunks using calculated index
int Game::getChunkIndex(int x, int y) {
    x = wrapCoordinate(x, WORLD_CHUNKS_X);
    y = wrapCoordinate(y, WORLD_CHUNKS_Y);
    return y * WORLD_CHUNKS_X + x;
}

// Main game loop
void Game::run() {
    while (window.isOpen()) {
        processEvents();
        render();
    }
}

// Function to wrap the view's center
void Game::wrapView() {
    float worldSizeX = WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE;
    float worldSizeY = WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE;

    sf::Vector2f center = view.getCenter();

    // Wrap the center coordinates
    center.x = wrapFloat(center.x, worldSizeX);
    center.y = wrapFloat(center.y, worldSizeY);

    view.setCenter(center);
}

// Process user input events
void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        // Close the window if the close button is pressed
        if (event.type == sf::Event::Closed) {
            window.close();
        }
    }

    // Handle input for camera movement
    const float cameraSpeed = 10.0f;

    // Move the view based on keyboard input
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        view.move(-cameraSpeed, 0);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        view.move(cameraSpeed, 0);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        view.move(0, -cameraSpeed);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        view.move(0, cameraSpeed);
    }

    wrapView();
}

int Game::wrapCoordinate(int coord, int max) {
    return (coord % max + max) % max;
}

// Utility function to calculate the number of steps needed to iterate from start to end with wrapping
int Game::calculateSteps(int start, int end, int max) {
    if (end >= start) {
        return end - start + 1;
    }
    else {
        return (max - start) + (end + 1);
    }
}

// Generate a chunk of tiles
Chunk Game::generateChunk(int chunkX, int chunkY) {
    chunkX = wrapCoordinate(chunkX, WORLD_CHUNKS_X);
    chunkY = wrapCoordinate(chunkY, WORLD_CHUNKS_Y);

    Chunk chunk;
    int totalTilesX = WORLD_CHUNKS_X * CHUNK_SIZE;
    int totalTilesY = WORLD_CHUNKS_Y * CHUNK_SIZE;

    // Calculate tile size in float for positioning
    float tileSizeF = static_cast<float>(TILE_SIZE);

    // Generate a chunk of tiles at the specified chunk coordinates
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            // Calculate tile's world position
            float tilePosX = static_cast<float>((chunkX * CHUNK_SIZE + x) * TILE_SIZE);
            float tilePosY = static_cast<float>((chunkY * CHUNK_SIZE + y) * TILE_SIZE);

            sf::Color color;
            if (chunkY == WORLD_CHUNKS_Y / 2) {
                color = sf::Color::Green;
            }
            else {
                // Alternate between land and water tiles to create a checkerboard pattern
                if (((chunkX * CHUNK_SIZE + x) + (chunkY * CHUNK_SIZE + y)) % 2 == 0) {
                    color = sf::Color::Green;
                }
                else {
                    color = sf::Color::Blue;
                }
            }

            // Define the four corners of the quad
            sf::Vertex topLeft(sf::Vector2f(tilePosX, tilePosY), color);
            sf::Vertex topRight(sf::Vector2f(tilePosX + tileSizeF, tilePosY), color);
            sf::Vertex bottomRight(sf::Vector2f(tilePosX + tileSizeF, tilePosY + tileSizeF), color);
            sf::Vertex bottomLeft(sf::Vector2f(tilePosX, tilePosY + tileSizeF), color);

            // Append the quad to the vertex array
            chunk.vertices.append(topLeft);
            chunk.vertices.append(topRight);
            chunk.vertices.append(bottomRight);
            chunk.vertices.append(bottomLeft);
        }
    }
    return chunk;
}

// Render the game scene
void Game::render() {
    // Clear the window with a black color
    window.clear(sf::Color::Black);
    // Set the view for rendering
    window.setView(view);

    // Determine which chunks need to be rendered based on the view
    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();

    // Calculate world size in pixels
    float worldSizeX = WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE;
    float worldSizeY = WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE;

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
            int baseChunkX = firstChunkX + dx * WORLD_CHUNKS_X;
            int baseChunkY = firstChunkY + dy * WORLD_CHUNKS_Y;

            // Iterate through all visible chunks
            for (int y = 0; y < visibleChunksY; ++y) {
                for (int x = 0; x < visibleChunksX; ++x) {
                    int chunkX = baseChunkX + x;
                    int chunkY = baseChunkY + y;

                    // Wrap the chunk indices
                    int wrappedChunkX = wrapCoordinate(chunkX, WORLD_CHUNKS_X);
                    int wrappedChunkY = wrapCoordinate(chunkY, WORLD_CHUNKS_Y);

                    int index = getChunkIndex(wrappedChunkX, wrappedChunkY);

                    if (index >= 0 && index < chunks.size()) {
                        const Chunk& currentChunk = chunks[index];

                        // Calculate the position shift based on the grid offset
                        float shiftX = dx * worldSizeX;
                        float shiftY = dy * worldSizeY;

                        // Create a transform for the shifted position
                        sf::Transform transform;
                        transform.translate(shiftX, shiftY);

                        // Draw the chunk with the applied transform
                        window.draw(currentChunk.vertices, transform);

                        // Optionally, draw chunk outlines for debugging
                        /*
                        sf::RectangleShape outline(sf::Vector2f(CHUNK_SIZE * TILE_SIZE, CHUNK_SIZE * TILE_SIZE));
                        outline.setFillColor(sf::Color::Transparent);
                        outline.setOutlineColor(sf::Color::Red);
                        outline.setOutlineThickness(1.0f);
                        outline.setPosition((chunkX % WORLD_CHUNKS_X) * CHUNK_SIZE * TILE_SIZE + shiftX,
                                           (chunkY % WORLD_CHUNKS_Y) * CHUNK_SIZE * TILE_SIZE + shiftY);
                        window.draw(outline);
                        */
                    }
                }
            }
        }
    }

    // Display the rendered frame
    window.display();
}
