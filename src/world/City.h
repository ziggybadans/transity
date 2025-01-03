#pragma once
#include <string>
#include <SFML/Graphics.hpp>

struct City {
	std::string name;
	sf::Vector2f position;
	unsigned int population;

	float radius;
	bool selected;

	City(const std::string& cityName,
		const sf::Vector2f& cityPosition,
		unsigned int cityPopulation,
		float cityRadius = 10.0f)
		: name(cityName),
		position(cityPosition),
		population(cityPopulation),
		radius(cityRadius),
		selected(false) {}

	bool operator==(const City& other) const {
		return name == other.name && position == other.position;
	}
};