#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>

class Node {
public:
    virtual ~Node() = default;

    Node(const std::string& nodeName, const sf::Vector2f& nodePosition, float nodeRadius = 5.0f)
        : name(nodeName), position(nodePosition), radius(nodeRadius), selected(false) {}

    nlohmann::json Serialize() const {
        nlohmann::json j;
        j["name"] = GetName();
        j["position"] = { position.x, position.y };
        j["radius"] = radius;
        j["selected"] = selected;
        return j;
    }
    void Deserialize(const nlohmann::json& j) {
        // Assuming setters or direct access to members
        // Note: If Node's fields are private and without setters, adjust accordingly.
        // Here, we assume direct setting is possible.
        // Since Node doesn't have setters for name and radius in provided code,
        // you might need to modify class design to allow deserialization.
        name = j["name"].get<std::string>();
        auto pos = j["position"];
        position = sf::Vector2f(pos[0], pos[1]);
        radius = j["radius"].get<float>();
        selected = j["selected"].get<bool>();
    }

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