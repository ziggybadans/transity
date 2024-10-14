// src/Chunk.h
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct ChunkCoord {
    int x;
    int y;

    bool operator==(const ChunkCoord& other) const {
        return x == other.x && y == other.y;
    }
};

struct Chunk {
    sf::VertexArray vertices;

    Chunk() : vertices(sf::Quads) {}
};

// Specialize std::hash to allow ChunkCoord to be used as a key in unordered_map
namespace std {
    template <>
    struct hash<ChunkCoord> {
        std::size_t operator()(const ChunkCoord& coord) const noexcept {
            // A simple hash combination for two integers
            return std::hash<int>()(coord.x) ^ (std::hash<int>()(coord.y) << 1);
        }
    };
}