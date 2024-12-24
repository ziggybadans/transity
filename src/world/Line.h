#pragma once
#include <string>
#include <SFML/Graphics.hpp>

#include "City.h"

class Line {
public:
	Line(City* startCity, const std::string& lineName, const sf::Color& lineColor = sf::Color::Blue, float lineThickness = 8.0f)
		: name(lineName), color(lineColor), thickness(lineThickness) {
		cities.push_back(startCity);
	}

	void AddCity(City* city) {
		cities.push_back(city);
	}

	const std::vector<City*>& GetCities() const {
		return cities;
	}

	void SetThickness(float newThickness) {
		thickness = newThickness;
	}

	std::string name;
	sf::Color color;
	float thickness;

private:

	std::vector<City*> cities;
};