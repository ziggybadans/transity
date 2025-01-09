#pragma once
#include <string>
#include <SFML/Graphics.hpp>

class Node {
public:
    virtual ~Node() = default;

    Node(const std::string& nodeName, const sf::Vector2f& nodePosition, float nodeRadius = 5.0f)
        : name(nodeName), position(nodePosition), radius(nodeRadius), selected(false) {}

    // Getters
    std::string GetName() const { return name; }
    sf::Vector2f GetPosition() const { return position; }
    float GetRadius() const { return radius; }
    bool IsSelected() const { return selected; }

    // Setters
    void SetPosition(const sf::Vector2f& pos) { position = pos; }
    void SetSelected(bool value) { selected = value; }

private:
    std::string name;
    sf::Vector2f position;
    float radius;
    bool selected;
};