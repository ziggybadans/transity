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
            return h1 ^ (h2 * 0x9e3779b9); // Better hash combination to reduce collisions
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
    sf::VertexArray verticesLOD3; // Lower Detail
    sf::VertexArray verticesLOD4; // Lowest Detail
    sf::VertexArray contourLines;  // Contour Lines (currently unused)

    // Add flags to track if LODs need updating
    bool needsUpdateLOD0 = true;
    bool needsUpdateLOD1 = true;
    bool needsUpdateLOD2 = true;
    bool needsUpdateLOD3 = true;
    bool needsUpdateLOD4 = true;

    // Constructor
    Chunk();

    // Clears chunk data
    void clear();
};
