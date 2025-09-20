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

// Holds the vertex arrays for a chunk's different levels of detail.
struct ChunkMeshComponent {
    std::vector<sf::VertexArray> lodVertexArrays;

    ChunkMeshComponent() {
        lodVertexArrays.resize(static_cast<size_t>(LODLevel::Count));
        for (auto &va : lodVertexArrays) {
            va.setPrimitiveType(sf::PrimitiveType::Triangles);
        }
    }
};

// Tag component to indicate that a passenger's path should be visualized.
struct VisualizePathComponent {};