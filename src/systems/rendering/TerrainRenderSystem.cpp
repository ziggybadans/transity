#include "TerrainRenderSystem.h"
#include "components/RenderComponents.h"
#include "components/WorldComponents.h"
#include "world/WorldData.h"
#include "systems/gameplay/CityPlacementSystem.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

TerrainRenderSystem::TerrainRenderSystem(ColorManager &colorManager)
    : _colorManager(colorManager) {}

void TerrainRenderSystem::updateMeshes(entt::registry &registry, const WorldGenParams &worldParams) {
    auto view = registry.view<ChunkPositionComponent, ChunkTerrainComponent, ChunkElevationComponent,
                              ChunkStateComponent, ChunkMeshComponent>();

    for (auto entity : view) {
        auto &chunkState = view.get<ChunkStateComponent>(entity);
        if (chunkState.isMeshDirty) {
            auto &chunkPos = view.get<ChunkPositionComponent>(entity);
            auto &chunkTerrain = view.get<ChunkTerrainComponent>(entity);
            auto &chunkElevation = view.get<ChunkElevationComponent>(entity);
            auto &chunkMesh = view.get<ChunkMeshComponent>(entity);
            // Pass worldParams instead of worldGrid
            buildAllChunkMeshes(chunkPos, chunkTerrain, chunkElevation, chunkMesh, worldParams);
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
                                              const ChunkElevationComponent &chunkElevation,
                                              ChunkMeshComponent &chunkMesh,
                                              const WorldGenParams &worldParams) {
    if (_shadedReliefEnabled) {
        buildChunkMeshShaded(chunkPos, chunkTerrain, chunkElevation, chunkMesh, worldParams);
    } else {
        buildChunkMeshMerged(chunkPos, chunkTerrain, chunkMesh, worldParams);
    }
}

void TerrainRenderSystem::buildChunkMeshMerged(const ChunkPositionComponent &chunkPos,
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

void TerrainRenderSystem::buildChunkMeshShaded(const ChunkPositionComponent &chunkPos,
                                               const ChunkTerrainComponent &chunkTerrain,
                                               const ChunkElevationComponent &chunkElevation,
                                               ChunkMeshComponent &chunkMesh,
                                               const WorldGenParams &worldParams) {
    const int cellsX = worldParams.chunkDimensionsInCells.x;
    const int cellsY = worldParams.chunkDimensionsInCells.y;

    sf::VertexArray &vertexArray = chunkMesh.vertexArray;
    vertexArray.clear();
    vertexArray.setPrimitiveType(sf::PrimitiveType::Triangles);

    const std::vector<float> &elevations = chunkElevation.elevations;
    auto elevationAt = [&](int cellX, int cellY, float fallback) -> float {
        if (cellX < 0 || cellY < 0 || cellX >= cellsX || cellY >= cellsY) {
            return fallback;
        }
        const size_t index = static_cast<size_t>(cellY * cellsX + cellX);
        if (index < elevations.size()) {
            return elevations[index];
        }
        return fallback;
    };

    const float maxElevation = std::max(0.0001f, worldParams.elevation.maxElevation);
    const float cellSize = worldParams.cellSize;

    sf::Vector3f lightDir(-0.5f, -0.7f, 1.0f);
    const float lightLength = std::sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y
                                        + lightDir.z * lightDir.z);
    if (lightLength > 0.0f) {
        lightDir.x /= lightLength;
        lightDir.y /= lightLength;
        lightDir.z /= lightLength;
    }

    for (int y = 0; y < cellsY; ++y) {
        for (int x = 0; x < cellsX; ++x) {
            const size_t cellIndex = static_cast<size_t>(y * cellsX + x);
            const TerrainType terrainType =
                (cellIndex < chunkTerrain.cells.size()) ? chunkTerrain.cells[cellIndex]
                                                        : TerrainType::WATER;
            const float centerElevation =
                (cellIndex < elevations.size()) ? elevations[cellIndex] : 0.0f;
            float normalizedElevation = centerElevation / maxElevation;
            normalizedElevation = std::clamp(normalizedElevation, 0.0f, 1.0f);

            const float leftElevation = elevationAt(x - 1, y, centerElevation);
            const float rightElevation = elevationAt(x + 1, y, centerElevation);
            const float upElevation = elevationAt(x, y - 1, centerElevation);
            const float downElevation = elevationAt(x, y + 1, centerElevation);

            const float dx = (rightElevation - leftElevation) / (2.0f * cellSize);
            const float dy = (downElevation - upElevation) / (2.0f * cellSize);

            sf::Vector3f normal(-dx, -dy, 1.0f);
            const float normalLength = std::sqrt(normal.x * normal.x + normal.y * normal.y
                                                 + normal.z * normal.z);
            if (normalLength > 0.0f) {
                normal.x /= normalLength;
                normal.y /= normalLength;
                normal.z /= normalLength;
            }

            const float diffuse =
                std::max(0.0f, normal.x * lightDir.x + normal.y * lightDir.y + normal.z * lightDir.z);
            float lightingFactor = 0.35f + 0.55f * diffuse + 0.25f * normalizedElevation;
            lightingFactor = std::clamp(lightingFactor, 0.25f, 1.3f);

            const sf::Color color =
                shadeColorForTerrain(terrainType, normalizedElevation, lightingFactor);

            const float screenX =
                (chunkPos.chunkGridPosition.x * cellsX + x) * worldParams.cellSize;
            const float screenY =
                (chunkPos.chunkGridPosition.y * cellsY + y) * worldParams.cellSize;

            sf::Vertex quad[6];
            quad[0].position = {screenX, screenY};
            quad[1].position = {screenX + cellSize, screenY};
            quad[2].position = {screenX, screenY + cellSize};
            quad[3].position = {screenX + cellSize, screenY};
            quad[4].position = {screenX + cellSize, screenY + cellSize};
            quad[5].position = {screenX, screenY + cellSize};

            for (auto &vertex : quad) {
                vertex.color = color;
                vertexArray.append(vertex);
            }
        }
    }
}

sf::Color TerrainRenderSystem::shadeColorForTerrain(TerrainType type, float normalizedElevation,
                                                    float lightingFactor) const {
    sf::Color baseColor;
    switch (type) {
    case TerrainType::WATER:
        baseColor = _colorManager.getWaterColor();
        break;
    case TerrainType::LAND:
        baseColor = _colorManager.getLandColor();
        break;
    case TerrainType::RIVER:
        baseColor = _colorManager.getRiverColor();
        break;
    default:
        baseColor = sf::Color::Magenta;
        break;
    }

    float factor = lightingFactor;
    if (type == TerrainType::LAND) {
        factor = std::clamp(factor + normalizedElevation * 0.15f, 0.25f, 1.35f);
    } else if (type == TerrainType::WATER) {
        // Keep water shading subtle and closer to base color.
        factor = std::clamp(0.8f + (factor - 0.8f) * 0.4f, 0.6f, 1.05f);
    } else if (type == TerrainType::RIVER) {
        factor = std::clamp(factor * 0.9f, 0.4f, 1.1f);
    }

    auto applyFactor = [factor](std::uint8_t component) {
        return static_cast<std::uint8_t>(
            std::clamp(static_cast<float>(component) * factor, 0.0f, 255.0f));
    };

    return {applyFactor(baseColor.r), applyFactor(baseColor.g), applyFactor(baseColor.b),
            baseColor.a};
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
