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
    for (int y = 0; y < WORLD_CHUNKS_Y; ++y) {
        for (int x = 0; x < WORLD_CHUNKS_X; ++x) {
            ChunkCoord coord = { x, y };
            chunks[coord] = generateChunk(x, y);
        }
    }
}

// Destructor
Game::~Game() {
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
    const float cameraSpeed = 20.0f;

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

    // Generate a chunk of tiles at the specified chunk coordinates
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            sf::RectangleShape tile(sf::Vector2f(static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE)));

            // Calculate tile's world position without additional wrapping
            // Since the world is already wrapped by chunk indices, direct positioning suffices
            float tilePosX = static_cast<float>((chunkX * CHUNK_SIZE + x) * TILE_SIZE);
            float tilePosY = static_cast<float>((chunkY * CHUNK_SIZE + y) * TILE_SIZE);
            tile.setPosition(tilePosX, tilePosY);

            // Create a line of land tiles in the middle of the world
            if (chunkY == WORLD_CHUNKS_Y / 2) {
                tile.setFillColor(sf::Color::Green);
            }
            else {
                // Alternate between land and water tiles to create a checkerboard pattern
                if (((chunkX * CHUNK_SIZE + x) + (chunkY * CHUNK_SIZE + y)) % 2 == 0) {
                    tile.setFillColor(sf::Color::Green); // Land
                }
                else {
                    tile.setFillColor(sf::Color::Blue);  // Water
                }
            }

            chunk.tiles.push_back(tile);
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
    float rightBound = center.x + size.x / 2;
    float topBound = center.y - size.y / 2;
    float bottomBound = center.y + size.y / 2;

    // Convert bounds to chunk indices, adding a buffer of 1 to cover partially visible chunks
    int left = static_cast<int>(std::floor(leftBound / (CHUNK_SIZE * TILE_SIZE))) - 1;
    int right = static_cast<int>(std::floor(rightBound / (CHUNK_SIZE * TILE_SIZE))) + 1;
    int top = static_cast<int>(std::floor(topBound / (CHUNK_SIZE * TILE_SIZE))) - 1;
    int bottom = static_cast<int>(std::floor(bottomBound / (CHUNK_SIZE * TILE_SIZE))) + 1;

    // Wrap the coordinates to stay within the world bounds
    left = wrapCoordinate(left, WORLD_CHUNKS_X);
    right = wrapCoordinate(right, WORLD_CHUNKS_X);
    top = wrapCoordinate(top, WORLD_CHUNKS_Y);
    bottom = wrapCoordinate(bottom, WORLD_CHUNKS_Y);

    // Calculate the number of chunks to iterate over
    int stepsY = calculateSteps(top, bottom, WORLD_CHUNKS_Y) + 1;
    int stepsX = calculateSteps(left, right, WORLD_CHUNKS_X) + 1;

    // Determine if the view is near the edges based on half the view size
    bool nearLeft = (center.x - size.x / 2) < (CHUNK_SIZE * TILE_SIZE / 2);
    bool nearRight = (center.x + size.x / 2) > (worldSizeX - (CHUNK_SIZE * TILE_SIZE / 2));
    bool nearTop = (center.y - size.y / 2) < (CHUNK_SIZE * TILE_SIZE / 2);
    bool nearBottom = (center.y + size.y / 2) > (worldSizeY - (CHUNK_SIZE * TILE_SIZE / 2));

    // Render all the tiles in the visible chunks
    for (int i = 0; i < stepsY; ++i) {
        int chunkY = (top + i) % WORLD_CHUNKS_Y;
        for (int j = 0; j < stepsX; ++j) {
            int chunkX = (left + j) % WORLD_CHUNKS_X;
            ChunkCoord coord = { chunkX, chunkY };

            // Access the chunk directly without locking since all chunks are pre-generated
            auto it = chunks.find(coord);
            if (it != chunks.end()) {
                const Chunk& currentChunk = it->second;

                // Draw original chunk
                for (const auto& tile : currentChunk.tiles) {
                    window.draw(tile);
                }

                // Always draw wrapped duplicates
                sf::Transform transformRight;
                sf::Transform transformLeft;
                sf::Transform transformTop;
                sf::Transform transformBottom;

                transformRight.translate(worldSizeX, 0);
                transformLeft.translate(-worldSizeX, 0);
                transformTop.translate(0, -worldSizeY);
                transformBottom.translate(0, worldSizeY);

                // Draw horizontally wrapped duplicates
                for (const auto& tile : currentChunk.tiles) {
                    window.draw(tile, transformRight);
                    window.draw(tile, transformLeft);
                }

                // Draw vertically wrapped duplicates
                for (const auto& tile : currentChunk.tiles) {
                    window.draw(tile, transformTop);
                    window.draw(tile, transformBottom);
                }

                // Draw corner wrapped duplicates
                sf::Transform transformRightBottom;
                sf::Transform transformRightTop;
                sf::Transform transformLeftBottom;
                sf::Transform transformLeftTop;

                transformRightBottom.translate(worldSizeX, worldSizeY);
                transformRightTop.translate(worldSizeX, -worldSizeY);
                transformLeftBottom.translate(-worldSizeX, worldSizeY);
                transformLeftTop.translate(-worldSizeX, -worldSizeY);

                for (const auto& tile : currentChunk.tiles) {
                    window.draw(tile, transformRightBottom);
                    window.draw(tile, transformRightTop);
                    window.draw(tile, transformLeftBottom);
                    window.draw(tile, transformLeftTop);
                }
            }
        }
    }


    // Display the rendered frame
    window.display();
}
