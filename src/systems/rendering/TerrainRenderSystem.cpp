#include "TerrainRenderSystem.h"
#include "components/RenderComponents.h"
#include "components/WorldComponents.h"
#include "world/WorldData.h"
#include "systems/gameplay/CityPlacementSystem.h"
#include "Logger.h"
#include <cstdint>

TerrainRenderSystem::TerrainRenderSystem(ColorManager &colorManager)
    : _colorManager(colorManager) {}

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
    auto chunkView = registry.view<const ChunkPositionComponent, const ChunkMeshComponent>();

    sf::FloatRect viewBounds({view.getCenter() - view.getSize() / 2.f, view.getSize()});
    // Update to use worldParams
    viewBounds.position.x -= worldParams.cellSize;
    viewBounds.position.y -= worldParams.cellSize;
    viewBounds.size.x += worldParams.cellSize * 2;
    viewBounds.size.y += worldParams.cellSize * 2;

    for (auto entity : chunkView) {
        const auto &chunkPos = chunkView.get<const ChunkPositionComponent>(entity);
        const auto &chunkMesh = chunkView.get<const ChunkMeshComponent>(entity);

        float chunkWidthPixels = worldParams.chunkDimensionsInCells.x * worldParams.cellSize;
        float chunkHeightPixels = worldParams.chunkDimensionsInCells.y * worldParams.cellSize;
        sf::FloatRect chunkBounds({chunkPos.chunkGridPosition.x * chunkWidthPixels,
                                   chunkPos.chunkGridPosition.y * chunkHeightPixels},
                                  {chunkWidthPixels, chunkHeightPixels});

        if (!viewBounds.findIntersection(chunkBounds)) {
            continue;
        }

        target.draw(chunkMesh.vertexArray);

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
    const int cellsX = worldParams.chunkDimensionsInCells.x;
    const int cellsY = worldParams.chunkDimensionsInCells.y;

    sf::VertexArray &vertexArray = chunkMesh.vertexArray;
    vertexArray.clear();

    m_visited.assign(static_cast<size_t>(cellsX * cellsY), false);

    for (int y = 0; y < cellsY; ++y) {
        for (int x = 0; x < cellsX; ++x) {
            if (m_visited[static_cast<size_t>(y * cellsX + x)]) {
                continue;
            }

            const int originalCellIndex = y * cellsX + x;
            const TerrainType currentType = chunkTerrain.cells[originalCellIndex];

            int rectWidth = 1;
            while (x + rectWidth < cellsX) {
                const int nextIndex = y * cellsX + (x + rectWidth);
                if (m_visited[static_cast<size_t>(nextIndex)]
                    || chunkTerrain.cells[nextIndex] != currentType) {
                    break;
                }
                ++rectWidth;
            }

            int rectHeight = 1;
            while (y + rectHeight < cellsY) {
                bool canExtend = true;
                for (int i = 0; i < rectWidth; ++i) {
                    const int nextIndex = (y + rectHeight) * cellsX + (x + i);
                    if (m_visited[static_cast<size_t>(nextIndex)]
                        || chunkTerrain.cells[nextIndex] != currentType) {
                        canExtend = false;
                        break;
                    }
                }
                if (!canExtend) {
                    break;
                }
                ++rectHeight;
            }

            for (int ry = 0; ry < rectHeight; ++ry) {
                for (int rx = 0; rx < rectWidth; ++rx) {
                    m_visited[static_cast<size_t>((y + ry) * cellsX + (x + rx))] = true;
                }
            }

            const float screenX = (chunkPos.chunkGridPosition.x * cellsX + x) * worldParams.cellSize;
            const float screenY = (chunkPos.chunkGridPosition.y * cellsY + y) * worldParams.cellSize;
            const float quadWidth = static_cast<float>(rectWidth) * worldParams.cellSize;
            const float quadHeight = static_cast<float>(rectHeight) * worldParams.cellSize;

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
                color = _colorManager.getWaterColor();
                break;
            case TerrainType::LAND:
                color = _colorManager.getLandColor();
                break;
            case TerrainType::RIVER:
                color = _colorManager.getRiverColor();
                break;
            default:
                color = sf::Color::Magenta;
                break;
            }

            for (auto &vertex : quad) {
                vertex.color = color;
                vertexArray.append(vertex);
            }
        }
    }
}

void TerrainRenderSystem::setSuitabilityMapData(const SuitabilityMaps* maps, const std::vector<TerrainType>* terrainCache, const WorldGenParams& worldParams) {
    _suitabilityMaps = maps;
    _terrainCache = terrainCache;
    _suitabilityMapsDirty = true;
}

void TerrainRenderSystem::regenerateSuitabilityMaps(const WorldGenParams &worldParams) {
    if (!_suitabilityMaps || !_terrainCache) {
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
            if (type != SuitabilityMapType::Water && _suitabilityMaps->water[i] > 0.0f) {
                if ((*_terrainCache)[i] == TerrainType::WATER) {
                    continue;
                }
            }

            float value = data[i];
            if (value >= 0) {
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
    regenerate(SuitabilityMapType::Noise, _suitabilityMaps->noise);
    regenerate(SuitabilityMapType::Final, _suitabilityMaps->final);
    regenerate(SuitabilityMapType::Town, _suitabilityMaps->townFinal);
    regenerate(SuitabilityMapType::Suburb, _suitabilityMaps->suburbFinal);

    _suitabilityMapsDirty = false;
}
