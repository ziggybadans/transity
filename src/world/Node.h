#pragma once
#include <SFML/Graphics.hpp>

struct Node {
	sf::Vector2f position;

	Node(const sf::Vector2f& nodePosition) : position(nodePosition) {}
};