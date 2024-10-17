// src/Chunk.h
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

// Structure representing chunk coordinates
struct ChunkCoord {
    int x;
    int y;

    bool operator==(const ChunkCoord& other) const {
        return x == other.x && y == other.y;
    }
};

// Specialize std::hash to allow ChunkCoord to be used as a key in unordered_map
namespace std {
    template <>
    struct hash<ChunkCoord> {
        std::size_t operator()(const ChunkCoord& coord) const noexcept {
            // Improved hash combination to reduce collisions
            std::size_t h1 = std::hash<int>()(coord.x);
            std::size_t h2 = std::hash<int>()(coord.y);
            // Use a better combination method, e.g., boost::hash_combine equivalent
            return h1 ^ (h2 << 1); // Example combination
        }
    };
}

// Chunk class containing vertex data for different LODs
class Chunk {
public:
    // Vertex arrays for different LOD levels
    sf::VertexArray verticesLOD0; // High Detail
    sf::VertexArray verticesLOD1; // Medium Detail
    sf::VertexArray verticesLOD2; // Low Detail
    sf::VertexArray contourLines;  // Contour Lines (currently unused)

    // Constructor
    Chunk();

    // Clears chunk data
    void clear();
};
