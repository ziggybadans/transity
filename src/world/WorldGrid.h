#pragma once

#include "../core/Components.h"
#include <vector>

class WorldGrid {
public:
    WorldGrid(int width, int height);
    GridCellComponent& getCell(int x, int y);
    const GridCellComponent& getCell(int x, int y) const;
    void setCellType(int x, int y, const GridCellComponent& cellData);
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
private:
    int _width;
    int _height;
    std::vector<std::vector<GridCellComponent>> _gridData;
};