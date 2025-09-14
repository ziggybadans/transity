#include "TerrainRenderSystem.h"

#include "components/RenderComponents.h"
#include "components/WorldComponents.h"
#include "world/WorldData.h"
#include "core/PerfTimer.h"
#include "systems/gameplay/CityPlacementSystem.h"
#include "Logger.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>

TerrainRenderSystem::TerrainRenderSystem() {}

void TerrainRenderSystem::updateMeshes(entt::registry &registry, const WorldGenParams &worldParams) {
    auto view = registry.view<ChunkPositionComponent, ChunkTerrainComponent, ChunkStateComponent,
                              ChunkMeshComponent>();

    for (auto entity : view) {
        auto &chunkState = view.get<ChunkStateComponent>(entity);
        if (chunkState.isMeshDirty) {
            auto &chunkPos = view.get<ChunkPositionComponent>(entity);
            auto &chunkTerrain = view.get<ChunkTerrainComponent>(entity);
            auto &chunkMesh = view.get<ChunkMeshComponent>(entity);
            // Pass worldParams instead of worldGrid
            buildAllChunkMeshes(chunkPos, chunkTerrain, chunkMesh, worldParams);
            chunkState.isMeshDirty = false;
        }
    }
}

void TerrainRenderSystem::render(const entt::registry &registry, sf::RenderTarget &target,
                                 const sf::View &view, const WorldGenParams &worldParams) {
    auto chunkView = registry.view<const ChunkPositionComponent, const ChunkStateComponent,
                                   const ChunkMeshComponent>();

    sf::FloatRect viewBounds({view.getCenter() - view.getSize() / 2.f, view.getSize()});
    // Update to use worldParams
    viewBounds.position.x -= worldParams.cellSize;
    viewBounds.position.y -= worldParams.cellSize;
    viewBounds.size.x += worldParams.cellSize * 2;
    viewBounds.size.y += worldParams.cellSize * 2;

    for (auto entity : chunkView) {
        const auto &chunkPos = chunkView.get<const ChunkPositionComponent>(entity);
        const auto &chunkState = chunkView.get<const ChunkStateComponent>(entity);
        const auto &chunkMesh = chunkView.get<const ChunkMeshComponent>(entity);

        float chunkWidthPixels = worldParams.chunkDimensionsInCells.x * worldParams.cellSize;
        float chunkHeightPixels = worldParams.chunkDimensionsInCells.y * worldParams.cellSize;
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

            for (unsigned int i = 1; i < worldParams.chunkDimensionsInCells.x; ++i) {
                float x = chunkLeft + i * worldParams.cellSize;
                gridLines.append({{x, chunkTop}, gridColor});
                gridLines.append({{x, chunkBottom}, gridColor});
            }
            for (unsigned int i = 1; i < worldParams.chunkDimensionsInCells.y; ++i) {
                float y = chunkTop + i * worldParams.cellSize;
                gridLines.append({{chunkLeft, y}, gridColor});
                gridLines.append({{chunkRight, y}, gridColor});
            }
            target.draw(gridLines);
        }
    }

    if (_visualizeSuitabilityMap && _suitabilityMaps != nullptr && _suitabilityMapType != SuitabilityMapType::None) {
        if (_suitabilityMapsDirty) {
            regenerateSuitabilityMaps(worldParams);
        }

        auto it = _suitabilityMapTextures.find(_suitabilityMapType);
        if (it != _suitabilityMapTextures.end() && it->second) {
            sf::Sprite suitabilityMapSprite(it->second->getTexture());
            suitabilityMapSprite.setScale({worldParams.cellSize, worldParams.cellSize});
            target.draw(suitabilityMapSprite);
        }
    }
}

void TerrainRenderSystem::buildAllChunkMeshes(const ChunkPositionComponent &chunkPos,
                                              const ChunkTerrainComponent &chunkTerrain,
                                              ChunkMeshComponent &chunkMesh,
                                              const WorldGenParams &worldParams) {
    int cellsPerDimension = worldParams.chunkDimensionsInCells.x;

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
                                * worldParams.cellSize;
                float screenY = (chunkPos.chunkGridPosition.y * cellsPerDimension + (y * step))
                                * worldParams.cellSize;
                float quadWidth = rectWidth * worldParams.cellSize * step;
                float quadHeight = rectHeight * worldParams.cellSize * step;

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

void TerrainRenderSystem::setLodEnabled(bool enabled) noexcept {
    _isLodEnabled = enabled;
}

void TerrainRenderSystem::setSuitabilityMapData(const SuitabilityMaps* maps, const WorldGenParams& worldParams) {
    _suitabilityMaps = maps;
    _suitabilityMapsDirty = true;
}

void TerrainRenderSystem::regenerateSuitabilityMaps(const WorldGenParams &worldParams) {
    if (!_suitabilityMaps) {
        return;
    }

    int mapWidth = worldParams.worldDimensionsInChunks.x * worldParams.chunkDimensionsInCells.x;
    int mapHeight = worldParams.worldDimensionsInChunks.y * worldParams.chunkDimensionsInCells.y;
    
    unsigned int textureWidth = mapWidth;
    unsigned int textureHeight = mapHeight;

    auto regenerate = [&](SuitabilityMapType type, const std::vector<float>& data) {
        auto& texturePtr = _suitabilityMapTextures[type];
        if (!texturePtr) {
            texturePtr = std::make_unique<sf::RenderTexture>();
            if (!texturePtr->resize({textureWidth, textureHeight})) {
                LOG_ERROR("TerrainRenderSystem", "Failed to create suitability map texture of size %u x %u", textureWidth, textureHeight);
                return;
            }
        }

        texturePtr->clear(sf::Color::Transparent);

        sf::VertexArray suitabilityTriangles(sf::PrimitiveType::Triangles);
        for (size_t i = 0; i < data.size(); ++i) {
            // For maps other than the water map itself, don't visualize anything on water tiles.
            // In the water map, non-zero values represent water.
            if (type != SuitabilityMapType::Water && _suitabilityMaps->water[i] > 0.0f) {
                continue;
            }

            float value = data[i];
            if (value > 0) {
                int x = i % mapWidth;
                int y = i / mapWidth;

                sf::Color color(static_cast<std::uint8_t>(255 * (1.0f - value)), static_cast<std::uint8_t>(255 * value), 0, 128);

                sf::Vertex v1, v2, v3, v4;
                v1.position = {(float)x, (float)y};
                v2.position = {(float)x + 1, (float)y};
                v3.position = {(float)x + 1, (float)y + 1};
                v4.position = {(float)x, (float)y + 1};
                v1.color = v2.color = v3.color = v4.color = color;

                suitabilityTriangles.append(v1);
                suitabilityTriangles.append(v2);
                suitabilityTriangles.append(v3);
                suitabilityTriangles.append(v3);
                suitabilityTriangles.append(v4);
                suitabilityTriangles.append(v1);
            }
        }
        texturePtr->draw(suitabilityTriangles);
        texturePtr->display();
    };

    regenerate(SuitabilityMapType::Water, _suitabilityMaps->water);
    regenerate(SuitabilityMapType::Expandability, _suitabilityMaps->expandability);
    regenerate(SuitabilityMapType::CityProximity, _suitabilityMaps->cityProximity);
    regenerate(SuitabilityMapType::Final, _suitabilityMaps->final);

    _suitabilityMapsDirty = false;
}