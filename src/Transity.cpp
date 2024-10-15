// src/main.cpp
#include "Game.h"

int main() {
    const int CHUNK_SIZE = 256; // World detail
    const int TILE_SIZE = 8;  // Pixelation
    const int WORLD_CHUNKS_X = 24; // World size
    const int WORLD_CHUNKS_Y = 12;

    Game game(CHUNK_SIZE, TILE_SIZE, WORLD_CHUNKS_X, WORLD_CHUNKS_Y);
    game.run();

    return 0;
}
