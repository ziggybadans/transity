#pragma once
#include <string>
#include <SFML/Graphics.hpp>

#include "City.h"

class Line {
public:
	Line(City* startCity, const std::string& lineName, const sf::Color& lineColor = sf::Color::Yellow)
		: name(lineName), color(lineColor) {
		cities.push_back(startCity);
	}

	void AddCity(City* city) {
		cities.push_back(city);
	}

	const std::vector<City*>& GetCities() const {
		return cities;
	}

	const sf::Color GetColor() const {
		return color;
	}

	std::string name;

private:
	sf::Color color;

	std::vector<City*> cities;
};