// QuadTree.h
#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <vector>
#include <memory>

class GeometryChunk;

class QuadTree {
public:
    QuadTree(const sf::FloatRect& bounds, int maxDepth = 8);

    void Insert(GeometryChunk* chunk);
    std::vector<GeometryChunk*> Query(const sf::FloatRect& area) const;

private:
    static constexpr int MAX_OBJECTS = 10;

    sf::FloatRect bounds;
    int depth;
    int maxDepth;

    std::vector<GeometryChunk*> objects;
    std::unique_ptr<QuadTree> northwest;
    std::unique_ptr<QuadTree> northeast;
    std::unique_ptr<QuadTree> southwest;
    std::unique_ptr<QuadTree> southeast;

    bool Subdivide();
    bool Contains(const sf::FloatRect& rect) const;
};