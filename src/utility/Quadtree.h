// QuadTree.h
#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <vector>
#include <memory>

class GeometryChunk;

class QuadTree {
public:
    // Constructor that initializes the quadtree with given bounds and maximum depth.
    QuadTree(const sf::FloatRect& bounds, int maxDepth = 8);

    // Inserts a GeometryChunk object into the appropriate quadrant.
    void Insert(GeometryChunk* chunk);

    // Queries the quadtree for all GeometryChunk objects that intersect the given area.
    std::vector<GeometryChunk*> Query(const sf::FloatRect& area) const;

private:
    static constexpr int MAX_OBJECTS = 10; // Maximum number of objects per node before subdivision.

    sf::FloatRect bounds; // The rectangular region that this quadtree node represents.
    int depth; // The current depth level of this node.
    int maxDepth; // The maximum depth the quadtree can reach.

    std::vector<GeometryChunk*> objects; // The list of objects contained in this node.
    std::unique_ptr<QuadTree> northwest; // Pointer to the northwest child node.
    std::unique_ptr<QuadTree> northeast; // Pointer to the northeast child node.
    std::unique_ptr<QuadTree> southwest; // Pointer to the southwest child node.
    std::unique_ptr<QuadTree> southeast; // Pointer to the southeast child node.

    // Subdivides the current node into four child nodes.
    bool Subdivide();

    // Checks if the provided rectangle is fully contained within the bounds of this node.
    bool Contains(const sf::FloatRect& rect) const;
};
