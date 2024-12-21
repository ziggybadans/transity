#pragma once
#include <vector>
#include <stdexcept>

#include "City.h"
#include "../Constants.h"

class Map {
public:
	Map(unsigned int size) : m_size(size), m_grid(size, std::vector<int>(size, 1)) {}

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
		if (pos.x < 0 || pos.y < 0) { return; }
		if (pos.x >= m_size * (Constants::TILE_SIZE * 0.98) || pos.y >= m_size * (Constants::TILE_SIZE * 0.98)) { return; }

		static int citySuffix = 1;
		std::string name = "City" + std::to_string(citySuffix++);
		unsigned int population = 1000;

		m_cities.emplace_back(name, pos, population);
	}

	std::vector<City> m_cities;

private:
	std::vector<std::vector<int>> m_grid;
	unsigned int m_size;

	void Resize(unsigned int newSize) {
		m_grid.resize(newSize, std::vector<int>(newSize, 1));
		for (auto& row : m_grid) {
			row.resize(newSize, 1);
		}
		m_size = newSize;
	}
};