#pragma once
#include <list>
#include <vector>
#include <SFML/Graphics.hpp>

class Line;
class Map;
struct Segment;
class Node;

class LineManager {
public:
    explicit LineManager(Map& map)
        : m_map(map) {}

    void UseLineMode(const sf::Vector2f& pos);
    void CreateLine(const sf::Vector2f& pos);
    void AddToLineStart(const sf::Vector2f& pos);
    void AddToLineEnd(const sf::Vector2f& pos);
    void RemoveLine();
    void MoveSelectedLineHandle(const sf::Vector2f& newPos);
    void UpdateSharedSegments();
    std::list<Line>& GetLines() { return m_lines; }
    std::vector<Segment> GetSharedSegments() const { return sharedSegments; }
    void CreateBranch(Line* parentLine, int branchHandleIndex, sf::Vector2f pos);

    std::vector<Node*> FindRouteBetweenNodes(Node* start, Node* end);

private:
    std::list<Line> m_lines;
    std::vector<Segment> sharedSegments;
    Map& m_map;

    static bool ArePositionsEqual(const sf::Vector2f& pos1, const sf::Vector2f& pos2, float epsilon = 0.1f) {
        return (std::abs(pos1.x - pos2.x) <= epsilon) && (std::abs(pos1.y - pos2.y) <= epsilon);
    }
};
