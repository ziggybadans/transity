// src/main.cpp
#include "Game.h"

int main() {
    const int CHUNK_SIZE = 512; // World detail - affects memory usage
    const int TILE_SIZE = 1;  // Pixelation - affects framerate
    const int WORLD_CHUNKS_X = 76; // World size
    const int WORLD_CHUNKS_Y = 48;

    Game game(CHUNK_SIZE, TILE_SIZE, WORLD_CHUNKS_X, WORLD_CHUNKS_Y);
    game.run();

    return 0;
}
