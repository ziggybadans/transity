// src/main.cpp
#include "Game.h"

int main() {
    const int CHUNK_SIZE = 16; // Example value
    const int TILE_SIZE = 32;  // Example value
    const int WORLD_CHUNKS_X = 10;
    const int WORLD_CHUNKS_Y = 10;

    Game game(CHUNK_SIZE, TILE_SIZE, WORLD_CHUNKS_X, WORLD_CHUNKS_Y);
    game.run();

    return 0;
}
