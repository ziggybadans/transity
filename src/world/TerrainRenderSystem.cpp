#include "TerrainRenderSystem.h"
#include <algorithm>
#include <cstdint>
#include <iostream>

TerrainRenderSystem::TerrainRenderSystem() {}

const WorldGridComponent &
TerrainRenderSystem::getWorldGridSettings(const entt::registry &registry) {
    auto view = registry.view<const WorldGridComponent>();
    if (view.empty()) {
        throw std::runtime_error("TerrainRenderSystem: WorldGridComponent not found in registry!");
    }
    return view.get<const WorldGridComponent>(view.front());
}

void TerrainRenderSystem::updateMeshes(entt::registry &registry) {
    const auto &worldGrid = getWorldGridSettings(registry);
    auto view = registry.view<ChunkPositionComponent, ChunkTerrainComponent, ChunkStateComponent, ChunkMeshComponent>();

    for (auto entity : view) {
        auto &chunkState = view.get<ChunkStateComponent>(entity);
        if (chunkState.isMeshDirty) {
            auto &chunkPos = view.get<ChunkPositionComponent>(entity);
            auto &chunkTerrain = view.get<ChunkTerrainComponent>(entity);
            auto &chunkMesh = view.get<ChunkMeshComponent>(entity);
            buildAllChunkMeshes(chunkPos, chunkTerrain, chunkMesh, worldGrid);
            chunkState.isMeshDirty = false;
        }
    }
}

void TerrainRenderSystem::render(const entt::registry &registry, sf::RenderTarget &target,
                                 const sf::View &view) {
    const WorldGridComponent &worldGrid = getWorldGridSettings(registry);
    auto chunkView = registry.view<const ChunkPositionComponent, const ChunkStateComponent, const ChunkMeshComponent>();

    sf::FloatRect viewBounds({view.getCenter() - view.getSize() / 2.f, view.getSize()});
    viewBounds.position.x -= worldGrid.cellSize;
    viewBounds.position.y -= worldGrid.cellSize;
    viewBounds.size.x += worldGrid.cellSize * 2;
    viewBounds.size.y += worldGrid.cellSize * 2;

    for (auto entity : chunkView) {
        const auto &chunkPos = chunkView.get<const ChunkPositionComponent>(entity);
        const auto &chunkState = chunkView.get<const ChunkStateComponent>(entity);
        const auto &chunkMesh = chunkView.get<const ChunkMeshComponent>(entity);

        float chunkWidthPixels = worldGrid.chunkDimensionsInCells.x * worldGrid.cellSize;
        float chunkHeightPixels = worldGrid.chunkDimensionsInCells.y * worldGrid.cellSize;
        sf::FloatRect chunkBounds({chunkPos.chunkGridPosition.x * chunkWidthPixels,
                                   chunkPos.chunkGridPosition.y * chunkHeightPixels},
                                  {chunkWidthPixels, chunkHeightPixels});

        if (!viewBounds.findIntersection(chunkBounds)) {
            continue;
        }

        LODLevel levelToRender = _isLodEnabled ? chunkState.lodLevel : LODLevel::LOD0;
        target.draw(chunkMesh.lodVertexArrays[static_cast<int>(levelToRender)]);

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
            border[4].position = {left, top};
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

            for (unsigned int i = 1; i < worldGrid.chunkDimensionsInCells.x; ++i) {
                float x = chunkLeft + i * worldGrid.cellSize;
                gridLines.append({{x, chunkTop}, gridColor});
                gridLines.append({{x, chunkBottom}, gridColor});
            }
            for (unsigned int i = 1; i < worldGrid.chunkDimensionsInCells.y; ++i) {
                float y = chunkTop + i * worldGrid.cellSize;
                gridLines.append({{chunkLeft, y}, gridColor});
                gridLines.append({{chunkRight, y}, gridColor});
            }
            target.draw(gridLines);
        }
    }
}

void TerrainRenderSystem::buildAllChunkMeshes(const ChunkPositionComponent &chunkPos,
                                              const ChunkTerrainComponent &chunkTerrain,
                                              ChunkMeshComponent &chunkMesh,
                                              const WorldGridComponent &worldGrid) {
    int cellsPerDimension = worldGrid.chunkDimensionsInCells.x;

    for (int lod = 0; lod < static_cast<int>(LODLevel::Count); ++lod) {
        int step = 1 << lod;
        sf::VertexArray &vertexArray = chunkMesh.lodVertexArrays[lod];
        vertexArray.clear();

        int numCellsX = cellsPerDimension / step;
        int numCellsY = cellsPerDimension / step;

        m_visited.assign(numCellsX * numCellsY, false);

        for (int y = 0; y < numCellsY; ++y) {
            for (int x = 0; x < numCellsX; ++x) {
                if (m_visited[y * numCellsX + x]) {
                    continue;
                }

                int originalCellIndex = (y * step) * cellsPerDimension + (x * step);
                TerrainType currentType = chunkTerrain.cells[originalCellIndex];

                int rectWidth = 1;
                while (x + rectWidth < numCellsX) {
                    int nextCellIndex = (y * step) * cellsPerDimension + ((x + rectWidth) * step);
                    if (m_visited[y * numCellsX + (x + rectWidth)]
                        || chunkTerrain.cells[nextCellIndex] != currentType) {
                        break;
                    }
                    rectWidth++;
                }

                int rectHeight = 1;
                while (y + rectHeight < numCellsY) {
                    bool canExtend = true;
                    for (int i = 0; i < rectWidth; ++i) {
                        int nextCellIndex =
                            ((y + rectHeight) * step) * cellsPerDimension + ((x + i) * step);
                        if (m_visited[(y + rectHeight) * numCellsX + (x + i)]
                            || chunkTerrain.cells[nextCellIndex] != currentType) {
                            canExtend = false;
                            break;
                        }
                    }
                    if (!canExtend) {
                        break;
                    }
                    rectHeight++;
                }

                for (int ry = 0; ry < rectHeight; ++ry) {
                    for (int rx = 0; rx < rectWidth; ++rx) {
                        m_visited[(y + ry) * numCellsX + (x + rx)] = true;
                    }
                }

                float screenX = (chunkPos.chunkGridPosition.x * cellsPerDimension + (x * step))
                                * worldGrid.cellSize;
                float screenY = (chunkPos.chunkGridPosition.y * cellsPerDimension + (y * step))
                                * worldGrid.cellSize;
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
                case TerrainType::WATER:
                    color = sf::Color(173, 216, 230);
                    break;
                case TerrainType::LAND:
                    color = sf::Color(34, 139, 34);
                    break;
                case TerrainType::RIVER:
                    color = sf::Color(100, 149, 237);
                    break;
                default:
                    color = sf::Color::Magenta;
                    break;
                }

                for (int i = 0; i < 6; ++i) {
                    quad[i].color = color;
                    vertexArray.append(quad[i]);
                }
            }
        }
    }
}

void TerrainRenderSystem::setLodEnabled(bool enabled) {
    _isLodEnabled = enabled;
}
