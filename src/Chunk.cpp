// src/Chunk.cpp
#include "Chunk.h"

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
