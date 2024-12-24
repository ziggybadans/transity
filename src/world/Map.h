#pragma once
#include <vector>
#include <stdexcept>

#include "City.h"
#include "Line.h"
#include "Node.h"
#include "../Constants.h"
#include "../Debug.h"

class Map {
public:
	Map(unsigned int size) : m_size(size), m_grid(size, std::vector<int>(size, 1)), m_minRadius(100), selectedLine(nullptr) {}

	void SetTile(unsigned int x, unsigned int y, int value) {
		if (x >= m_size || y >= m_size) throw std::out_of_range("Invalid tile coordinates");
		m_grid[x][y] = value;
	}

	int GetSize() const {
		return m_size;
	}

	int GetTile(int x, int y) const {
		return m_grid[x][y];
	}

	void AddCity(sf::Vector2f pos) {
		/* Validation checks */
		// Check inside map bounds
		if (pos.x < 0 || pos.y < 0) { return; }
		if (pos.x >= m_size * (Constants::TILE_SIZE * 0.98) || pos.y >= m_size * (Constants::TILE_SIZE * 0.98)) { return; }

		// Check outside minimum radius to another city
		for (City city : m_cities) {
			sf::Vector2f diff = city.position - pos;
			float distanceSquared = diff.x * diff.x + diff.y * diff.y;
			if (distanceSquared <= m_minRadius * m_minRadius) {
				return;
			}
		}

		/* Data generation*/
		static int citySuffix = 1;
		std::string name = "City" + std::to_string(citySuffix++);
		unsigned int population = 1000;

		m_cities.emplace_back(name, pos, population); // Adds to the list of cities
	}

	void UseLineMode(sf::Vector2f pos) {
		DEBUG_DEBUG("Choosing to either create new line or add to existing line.");
		if (selectedLine == nullptr) {
			CreateLine(pos);
		}
		else {
			AddToLine(pos);
		}
	}

	void CreateLine(sf::Vector2f pos) {
		DEBUG_DEBUG("Creating new line...");
		City* firstCity = nullptr;
		for (auto& city : m_cities) {
			sf::Vector2f diff = city.position - pos;
			float distanceSquared = diff.x * diff.x + diff.y * diff.y;
			if (distanceSquared <= m_minRadius * m_minRadius) {
				firstCity = &city;

				static int lineSuffix = 1;
				std::string name = "Line" + std::to_string(lineSuffix++);

				Line newLine = Line(firstCity, name);

				m_lines.emplace_back(newLine);
				selectedLine = &m_lines.back();

				DEBUG_DEBUG("New line created originating from " + firstCity->name + " with name " + name + ". Selected line has been updated for new line.");
				return;
			}
		}

		if (firstCity == nullptr) {
			DEBUG_DEBUG("You need to click on a city to create a line!");
		}
	}

	void AddToLine(sf::Vector2f pos) {
		DEBUG_DEBUG("Adding city to line " + selectedLine->name + "...");
		City* firstCity = nullptr;
		for (auto& city : m_cities) {
			sf::Vector2f diff = city.position - pos;
			float distanceSquared = diff.x * diff.x + diff.y * diff.y;
			if (distanceSquared <= m_minRadius * m_minRadius) {
				firstCity = &city;

				selectedLine->AddCity(firstCity);
				DEBUG_DEBUG("Added city with name " + firstCity->name + " to line with name " + selectedLine->name);
				return;
			}
		}

		if (firstCity == nullptr) {
			DEBUG_DEBUG("You need to click on a city to add one to the line!");
		}
	}

	void SelectLine(Line* line) {
		DEBUG_DEBUG("Line of name " + line->name + "has been selected.");
		selectedLine = line;
	}

	void DeselectLine() {
		DEBUG_DEBUG("No line is selected.");
		selectedLine = nullptr;
	}

	std::vector<City> m_cities;
	std::vector<Line> m_lines;
	Line* selectedLine;

private:
	std::vector<std::vector<int>> m_grid;
	unsigned int m_size;

	unsigned int m_minRadius;

	void Resize(unsigned int newSize) {
		m_grid.resize(newSize, std::vector<int>(newSize, 1));
		for (auto& row : m_grid) {
			row.resize(newSize, 1);
		}
		m_size = newSize;
	}
};