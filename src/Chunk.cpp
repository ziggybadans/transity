// src/Chunk.cpp
#include "Chunk.h"

Chunk::Chunk()
    : verticesLOD0(sf::Quads),
    verticesLOD1(sf::Quads),
    verticesLOD2(sf::Quads),
    contourLines(sf::Lines)
{
}

void Chunk::clear() {
    verticesLOD0.clear();
    verticesLOD1.clear();
    verticesLOD2.clear();
    contourLines.clear();
}
