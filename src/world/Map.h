#pragma once
#include <vector>
#include <stdexcept>

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