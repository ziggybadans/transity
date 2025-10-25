#pragma once

#include "StrongTypes.h"
#include "WorldComponents.h"
#include "components/GameLogicComponents.h"
#include <SFML/Graphics.hpp>

// General-purpose bounding box for collision detection and culling.
struct AABBComponent {
    sf::FloatRect bounds;
};

// Visual representation of an entity.
struct RenderableComponent {
    Radius radius;
    sf::Color color;
    ZOrder zOrder;
};

// Holds the vertex array for a chunk.
struct ChunkMeshComponent {
    sf::VertexArray vertexArray;

    ChunkMeshComponent() {
        vertexArray.setPrimitiveType(sf::PrimitiveType::Triangles);
    }
};

// Tag component to indicate that a passenger's path should be visualized.
struct VisualizePathComponent {};
