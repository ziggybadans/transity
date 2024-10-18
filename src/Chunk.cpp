// src/Chunk.cpp
#include "Chunk.h"
#include <memory> // For std::make_unique

Chunk::Chunk()
    : verticesLOD0(sf::Quads),
    verticesLOD1(sf::Quads),
    verticesLOD2(sf::Quads),
    verticesLOD3(sf::Quads), // Initialize LOD3
    verticesLOD4(sf::Quads), // Initialize LOD4
    contourLines(sf::Lines)
{
}

void Chunk::clear() {
    verticesLOD0.clear();
    verticesLOD1.clear();
    verticesLOD2.clear();
    verticesLOD3.clear(); // Clear LOD3
    verticesLOD4.clear(); // Clear LOD4
    contourLines.clear();
}

std::unique_ptr<Chunk> Chunk::clone() const {
    // Create a new Chunk instance
    auto newChunk = std::make_unique<Chunk>();

    // Copy each LOD vertex array
    newChunk->verticesLOD0 = this->verticesLOD0;
    newChunk->verticesLOD1 = this->verticesLOD1;
    newChunk->verticesLOD2 = this->verticesLOD2;
    newChunk->verticesLOD3 = this->verticesLOD3;
    newChunk->verticesLOD4 = this->verticesLOD4;

    // Copy the contour lines (if used)
    newChunk->contourLines = this->contourLines;

    // Copy the update flags
    newChunk->needsUpdateLOD0 = this->needsUpdateLOD0;
    newChunk->needsUpdateLOD1 = this->needsUpdateLOD1;
    newChunk->needsUpdateLOD2 = this->needsUpdateLOD2;
    newChunk->needsUpdateLOD3 = this->needsUpdateLOD3;
    newChunk->needsUpdateLOD4 = this->needsUpdateLOD4;

    // Return the deep copy of the chunk
    return newChunk;
}