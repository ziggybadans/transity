#include "TerrainRenderSystem.h"
#include <iostream> // For debugging if needed
#include <algorithm>
#include <cstdint>

TerrainRenderSystem::TerrainRenderSystem() {
    // Initialize cellShape once. Its size will be set from WorldGridComponent.
    // Its position and color will be set per cell.
}

const WorldGridComponent& TerrainRenderSystem::getWorldGridSettings(entt::registry& registry) {
    auto view = registry.view<WorldGridComponent>();
    if (view.empty()) {
        throw std::runtime_error("TerrainRenderSystem: WorldGridComponent not found in registry!");
    }
    return view.get<WorldGridComponent>(view.front());
}

void TerrainRenderSystem::render(entt::registry& registry, sf::RenderTarget& target, const sf::View& view) {
    const WorldGridComponent& worldGrid = getWorldGridSettings(registry);
    auto chunkView = registry.view<ChunkComponent>();

    // Calculate the view's bounding box using the SFML 3 API
    sf::FloatRect viewBounds({view.getCenter() - view.getSize() / 2.f, view.getSize()});

    // Optional: Inflate the view bounds slightly to prevent pop-in at the edges
    viewBounds.position.x -= worldGrid.cellSize;
    viewBounds.position.y -= worldGrid.cellSize;
    viewBounds.size.x += worldGrid.cellSize * 2;
    viewBounds.size.y += worldGrid.cellSize * 2;

    int renderedChunkCount = 0;
    for (auto entity : chunkView) {
        auto& chunk = chunkView.get<ChunkComponent>(entity);

        // Calculate the chunk's bounding box
        float chunkWidthPixels = worldGrid.chunkDimensionsInCells.x * worldGrid.cellSize;
        float chunkHeightPixels = worldGrid.chunkDimensionsInCells.y * worldGrid.cellSize;
        sf::FloatRect chunkBounds(
            {chunk.chunkGridPosition.x * chunkWidthPixels, chunk.chunkGridPosition.y * chunkHeightPixels},
            {chunkWidthPixels, chunkHeightPixels}
        );

        // The culling check remains the same, as intersects() is still the correct method
        if (!viewBounds.findIntersection(chunkBounds)) {
            continue;
        }

        // If the chunk is visible, proceed with rendering
        if (chunk.isMeshDirty) {
            buildAllChunkMeshes(chunk, worldGrid); // Call the new function
        }
        target.draw(chunk.lodVertexArrays[static_cast<int>(chunk.lodLevel)]);
        
        // --- START VISUALIZATION LOGIC ---
        if (_visualizeChunkBorders) {
            float left = chunkBounds.position.x;
            float top = chunkBounds.position.y;
            float right = left + chunkBounds.size.x;
            float bottom = top + chunkBounds.size.y;

            sf::VertexArray border(sf::PrimitiveType::LineStrip, 5);
            border[0].position = {left, top};
            border[0].color = sf::Color::Red;
            border[1].position = {right, top};
            border[1].color = sf::Color::Red;
            border[2].position = {right, bottom};
            border[2].color = sf::Color::Red;
            border[3].position = {left, bottom};
            border[3].color = sf::Color::Red;
            border[4].position = {left, top}; // Close the loop
            border[4].color = sf::Color::Red;

            target.draw(border);
        }

        if (_visualizeCellBorders) {
            sf::VertexArray gridLines(sf::PrimitiveType::Lines);
            float chunkLeft = chunkBounds.position.x;
            float chunkTop = chunkBounds.position.y;
            float chunkRight = chunkLeft + chunkBounds.size.x;
            float chunkBottom = chunkTop + chunkBounds.size.y;
            sf::Color gridColor(128, 128, 128, 128);

            // Vertical lines
            for (int i = 1; i < worldGrid.chunkDimensionsInCells.x; ++i) {
                float x = chunkLeft + i * worldGrid.cellSize;
                sf::Vertex topVertex, bottomVertex;
                
                topVertex.position = {x, chunkTop};
                topVertex.color = gridColor;
                
                bottomVertex.position = {x, chunkBottom};
                bottomVertex.color = gridColor;
                
                gridLines.append(topVertex);
                gridLines.append(bottomVertex);
            }
            // Horizontal lines
            for (int i = 1; i < worldGrid.chunkDimensionsInCells.y; ++i) {
                float y = chunkTop + i * worldGrid.cellSize;
                sf::Vertex leftVertex, rightVertex;

                leftVertex.position = {chunkLeft, y};
                leftVertex.color = gridColor;

                rightVertex.position = {chunkRight, y};
                rightVertex.color = gridColor;

                gridLines.append(leftVertex);
                gridLines.append(rightVertex);
            }
            target.draw(gridLines);
        }
        // --- END VISUALIZATION LOGIC ---

        renderedChunkCount++;
    }
    // LOG_TRACE("TerrainRenderSystem", "Rendered %d visible chunks.", renderedChunkCount);
}

void TerrainRenderSystem::buildAllChunkMeshes(ChunkComponent& chunk, const WorldGridComponent& worldGrid) {
    int cellsPerDimension = worldGrid.chunkDimensionsInCells.x;

    for (int lod = 0; lod < static_cast<int>(LODLevel::Count); ++lod) {
        int step = 1 << lod;
        sf::VertexArray& vertexArray = chunk.lodVertexArrays[lod];
        vertexArray.clear();

        int numCellsX = cellsPerDimension / step;
        int numCellsY = cellsPerDimension / step;
        std::vector<bool> visited(numCellsX * numCellsY, false);

        for (int y = 0; y < numCellsY; ++y) {
            for (int x = 0; x < numCellsX; ++x) {
                if (visited[y * numCellsX + x]) {
                    continue;
                }

                int originalCellIndex = (y * step) * cellsPerDimension + (x * step);
                TerrainType currentType = chunk.cells[originalCellIndex];

                // Find width of the rectangle
                int rectWidth = 1;
                while (x + rectWidth < numCellsX) {
                    int nextCellIndex = (y * step) * cellsPerDimension + ((x + rectWidth) * step);
                    if (visited[y * numCellsX + (x + rectWidth)] || chunk.cells[nextCellIndex] != currentType) {
                        break;
                    }
                    rectWidth++;
                }

                // Find height of the rectangle
                int rectHeight = 1;
                while (y + rectHeight < numCellsY) {
                    bool canExtend = true;
                    for (int i = 0; i < rectWidth; ++i) {
                        int nextCellIndex = ((y + rectHeight) * step) * cellsPerDimension + ((x + i) * step);
                        if (visited[(y + rectHeight) * numCellsX + (x + i)] || chunk.cells[nextCellIndex] != currentType) {
                            canExtend = false;
                            break;
                        }
                    }
                    if (!canExtend) {
                        break;
                    }
                    rectHeight++;
                }

                // Mark all cells in the rectangle as visited
                for (int ry = 0; ry < rectHeight; ++ry) {
                    for (int rx = 0; rx < rectWidth; ++rx) {
                        visited[(y + ry) * numCellsX + (x + rx)] = true;
                    }
                }

                // Add the quad for the rectangle
                float screenX = (chunk.chunkGridPosition.x * cellsPerDimension + (x * step)) * worldGrid.cellSize;
                float screenY = (chunk.chunkGridPosition.y * cellsPerDimension + (y * step)) * worldGrid.cellSize;
                float quadWidth = rectWidth * worldGrid.cellSize * step;
                float quadHeight = rectHeight * worldGrid.cellSize * step;

                sf::Vertex quad[6];
                quad[0].position = {screenX, screenY};
                quad[1].position = {screenX + quadWidth, screenY};
                quad[2].position = {screenX, screenY + quadHeight};
                quad[3].position = {screenX + quadWidth, screenY};
                quad[4].position = {screenX + quadWidth, screenY + quadHeight};
                quad[5].position = {screenX, screenY + quadHeight};

                sf::Color color;
                switch (currentType) {
                    case TerrainType::WATER: color = sf::Color(173, 216, 230); break;
                    case TerrainType::LAND:  color = sf::Color(34, 139, 34); break;
                    case TerrainType::RIVER: color = sf::Color(100, 149, 237); break;
                    default:                 color = sf::Color::Magenta; break;
                }

                for (int i = 0; i < 6; ++i) {
                    quad[i].color = color;
                    vertexArray.append(quad[i]);
                }
            }
        }
    }
    chunk.isMeshDirty = false;
}

void TerrainRenderSystem::setLodEnabled(entt::registry& registry, bool enabled) {
    if (_isLodEnabled == enabled) return;

    _isLodEnabled = enabled;

    auto chunkView = registry.view<ChunkComponent>();
    for (auto entity : chunkView) {
        auto& chunk = chunkView.get<ChunkComponent>(entity);
        chunk.isMeshDirty = true;
    }
}