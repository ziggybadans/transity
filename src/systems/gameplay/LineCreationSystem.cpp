#include "LineCreationSystem.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "systems/world/WorldGenerationSystem.h"
#include "ecs/EntityFactory.h"
#include "render/ColorManager.h"
#include "imgui.h"
#include "Constants.h"
#include "core/Curve.h"
#include "event/LineEvents.h"
#include <algorithm>
#include <utility>

LineCreationSystem::LineCreationSystem(entt::registry& registry, EntityFactory& entityFactory, ColorManager& colorManager, GameState& gameState, EventBus& eventBus, WorldGenerationSystem& worldGenerationSystem)
    : _registry(registry), _entityFactory(entityFactory),
      _colorManager(colorManager), _gameState(gameState), _eventBus(eventBus), _worldGenerationSystem(worldGenerationSystem) {

    m_finalizeLineConnection = _eventBus.sink<FinalizeLineEvent>()
                                   .connect<&LineCreationSystem::onFinalizeLine>(this);
    m_mousePressConnection = _eventBus.sink<MouseButtonPressedEvent>()
                                   .connect<&LineCreationSystem::onMouseButtonPressed>(this);
    m_cancelLineCreationConnection = _eventBus.sink<CancelLineCreationEvent>()
                                   .connect<&LineCreationSystem::onCancelLineCreation>(this);
    m_mouseMoveConnection = _eventBus.sink<MouseMovedEvent>()
                                  .connect<&LineCreationSystem::onMouseMoved>(this);
    _registry.ctx().emplace<LinePreview>();
    LOG_DEBUG("LineCreationSystem", "LineCreationSystem created and connected to EventBus.");
}

LineCreationSystem::~LineCreationSystem() {
    m_finalizeLineConnection.release();
    m_mousePressConnection.release();
    m_cancelLineCreationConnection.release();
    m_mouseMoveConnection.release();
    LOG_DEBUG("LineCreationSystem", "LineCreationSystem destroyed and disconnected from EventBus.");
}

void LineCreationSystem::onMouseMoved(const MouseMovedEvent &event) {
    _currentMouseWorldPos = event.worldPosition;
}

void LineCreationSystem::onMouseButtonPressed(const MouseButtonPressedEvent &event) {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    if (_gameState.currentInteractionMode == InteractionMode::CREATE_LINE && event.button == sf::Mouse::Button::Left) {
        auto& preview = _registry.ctx().get<LinePreview>();
        
        if (!preview.validSegments.empty()) {
            bool allSegmentsValid = true;
            for (bool isValid : preview.validSegments) {
                if (!isValid) {
                    allSegmentsValid = false;
                    break;
                }
            }
            if (!allSegmentsValid) {
                LOG_WARN("LineCreationSystem", "Cannot place point: the path to it crosses water.");
                return;
            }
        }

        if (preview.snapPosition && preview.snapInfo) {
            entt::entity stationEntity = entt::null;
            if (preview.snapInfo->snappedToPointIndex == (size_t)-1) {
                stationEntity = preview.snapInfo->snappedToEntity;
            }
            addPointToLine(*preview.snapPosition, stationEntity, preview.snapInfo, preview.snapSide);
            return;
        }

        auto view = _registry.view<PositionComponent, ClickableComponent, CityComponent>();
        for (auto entity_id : view) {
            const auto &pos = view.get<PositionComponent>(entity_id);
            const auto &clickable = view.get<ClickableComponent>(entity_id);
            sf::Vector2f diff = event.worldPosition - pos.coordinates;
            float distanceSquared = (diff.x * diff.x) + diff.y * diff.y;

            if (distanceSquared <= clickable.boundingRadius.value * clickable.boundingRadius.value) {
                addPointToLine(pos.coordinates, entity_id);
                return;
            }
        }

        addPointToLine(event.worldPosition);
    }
}

void LineCreationSystem::addPointToLine(const sf::Vector2f& position, entt::entity stationEntity, std::optional<SnapInfo> snapInfo, float snapSide) {
    if (_worldGenerationSystem.getTerrainTypeAt(position.x, position.y) == TerrainType::WATER) {
        LOG_WARN("LineCreationSystem", "Cannot place a line point on water.");
        return;
    }

    auto& activeLine = _registry.ctx().get<ActiveLine>();

    if (activeLine.points.empty() && stationEntity == entt::null) {
        LOG_WARN("LineCreationSystem", "The first point of a line must be a station.");
        return;
    }

    if (stationEntity != entt::null) {
        if (!activeLine.points.empty()) {
            const auto& lastPoint = activeLine.points.back();
            if (lastPoint.type == LinePointType::STOP && lastPoint.stationEntity == stationEntity) {
                LOG_WARN("LineCreationSystem", "Station %u is already the last point in the active line.", static_cast<unsigned int>(stationEntity));
                return;
            }
        }
        activeLine.points.push_back({LinePointType::STOP, position, stationEntity, snapInfo, snapSide});
        LOG_DEBUG("LineCreationSystem", "Station %u added to active line.", static_cast<unsigned int>(stationEntity));
    } else {
        activeLine.points.push_back({LinePointType::CONTROL_POINT, position, entt::null, snapInfo, snapSide});
        LOG_DEBUG("LineCreationSystem", "Control point added to active line at (%.1f, %.1f).", position.x, position.y);
    }
}

void LineCreationSystem::finalizeLine() {
    auto& activeLine = _registry.ctx().get<ActiveLine>();

    if (activeLine.points.size() < 2) {
        LOG_WARN("LineCreationSystem", "Not enough points to finalize line. Need at least 2, have %zu.", activeLine.points.size());
        clearCurrentLine();
        return;
    }

    if (activeLine.points.back().type != LinePointType::STOP) {
        LOG_WARN("LineCreationSystem", "Cannot finalize line: the last point must be a station.");
        return;
    }

    std::vector<sf::Vector2f> controlPointsForFinalize;
    for (const auto &point : activeLine.points) {
        controlPointsForFinalize.push_back(point.position);
    }
    CurveData finalCurveData = Curve::generateMetroCurve(controlPointsForFinalize, Constants::METRO_CURVE_RADIUS);

    for (size_t i = 0; i < finalCurveData.points.size() - 1; ++i) {
        const sf::Vector2f& p1 = finalCurveData.points[i];
        const sf::Vector2f& p2 = finalCurveData.points[i+1];
        sf::Vector2f midPoint = p1 + (p2 - p1) * 0.5f;
        
        if (_worldGenerationSystem.getTerrainTypeAt(midPoint.x, midPoint.y) == TerrainType::WATER) {
            LOG_WARN("LineCreationSystem", "Cannot finalize line: path intersects with water.");
            return;
        }
    }

    sf::Color chosenColor = _colorManager.getNextLineColor();
    entt::entity lineEntity = _entityFactory.createLine(activeLine.points, chosenColor);

    if (lineEntity == entt::null) {
        LOG_ERROR("LineCreationSystem", "Failed to create line entity.");
        clearCurrentLine();
        return;
    }

    auto& newLineComp = _registry.get<LineComponent>(lineEntity);
    auto& sharedSegmentsCtx = _registry.ctx().get<SharedSegmentsContext>();
    
    std::vector<sf::Vector2f> controlPoints;
    for (const auto &point : newLineComp.points) {
        controlPoints.push_back(point.position);
    }
    CurveData curveData = Curve::generateMetroCurve(controlPoints, Constants::METRO_CURVE_RADIUS);
    newLineComp.curvePoints = curveData.points;
    newLineComp.curveSegmentIndices = curveData.segmentIndices;
    newLineComp.totalDistance = Curve::calculateCurveLength(newLineComp.curvePoints);
    newLineComp.stops = Curve::calculateStopInfo(newLineComp.points, newLineComp.curvePoints);

    for (const auto& point : newLineComp.points) {
        if (point.type == LinePointType::STOP) {
            auto& stationComp = _registry.get<CityComponent>(point.stationEntity);
            stationComp.connectedLines.push_back(lineEntity);
        }
    }

    _eventBus.enqueue<LineModifiedEvent>({lineEntity});
    clearCurrentLine();
}

void LineCreationSystem::clearCurrentLine() noexcept {
    LOG_DEBUG("LineCreationSystem", "Clearing active line.");
    _registry.ctx().erase<ActiveLine>();
    _registry.ctx().emplace<ActiveLine>();
}

void LineCreationSystem::onFinalizeLine(const FinalizeLineEvent &event) {
    LOG_DEBUG("LineCreationSystem", "Processing FinalizeLineEvent.");
    finalizeLine();
}

void LineCreationSystem::update(sf::Time dt) {
    if (_gameState.currentInteractionMode != InteractionMode::CREATE_LINE) {
        return;
    }
    if (!_registry.ctx().contains<ActiveLine>()) {
        _registry.ctx().emplace<ActiveLine>();
    }

    auto& preview = _registry.ctx().get<LinePreview>();
    preview.snapPosition.reset();
    preview.snapInfo.reset();
    preview.snapSide = 0.f;
    preview.snapTangent.reset();
    preview.curvePoints.clear();
    preview.validSegments.clear();

    sf::Vector2f mousePos = _currentMouseWorldPos;

    const float SNAP_RADIUS_SQUARED = Constants::LINE_SNAP_RADIUS * Constants::LINE_SNAP_RADIUS;
    float closestDistSq = SNAP_RADIUS_SQUARED;

    auto lineView = _registry.view<const LineComponent>();
    for (auto entity : lineView) {
        const auto& lineComp = lineView.get<const LineComponent>(entity);
        for (size_t i = 0; i < lineComp.points.size(); ++i) {
            const auto& point = lineComp.points[i];
            if (point.type == LinePointType::CONTROL_POINT) {
                sf::Vector2f diff = mousePos - point.position;
                float distSq = diff.x * diff.x + diff.y * diff.y;
                if (distSq < closestDistSq) {
                    closestDistSq = distSq;
                    preview.snapInfo = SnapInfo{entity, i};
                }
            }
        }
    }

    auto cityView = _registry.view<const CityComponent, const PositionComponent>();
    for (auto entity : cityView) {
        const auto& pos = cityView.get<const PositionComponent>(entity);
        sf::Vector2f diff = mousePos - pos.coordinates;
        float distSq = diff.x * diff.x + diff.y * diff.y;
        if (distSq < closestDistSq) {
            closestDistSq = distSq;
            preview.snapInfo = SnapInfo{entity, (size_t)-1};
        }
    }

    sf::Vector2f finalPreviewPos = mousePos;

    if (preview.snapInfo) {
        sf::Vector2f targetPointPos;
        sf::Vector2f tangent;
        bool tangentFound = false;

        if (preview.snapInfo->snappedToPointIndex != (size_t)-1) {
            const auto& targetLine = _registry.get<LineComponent>(preview.snapInfo->snappedToEntity);
            const auto& targetPoint = targetLine.points[preview.snapInfo->snappedToPointIndex];
            targetPointPos = targetPoint.position;

            sf::Vector2f p_prev, p_next;
            if (preview.snapInfo->snappedToPointIndex > 0) {
                p_prev = targetLine.points[preview.snapInfo->snappedToPointIndex - 1].position;
            } else {
                p_prev = targetPoint.position - (targetLine.points[1].position - targetPoint.position);
            }

            if (preview.snapInfo->snappedToPointIndex < targetLine.points.size() - 1) {
                p_next = targetLine.points[preview.snapInfo->snappedToPointIndex + 1].position;
            } else {
                p_next = targetPoint.position + (targetPoint.position - targetLine.points[targetLine.points.size() - 2].position);
            }
            tangent = p_next - p_prev;
            tangentFound = true;
        } else {
            entt::entity cityEntity = preview.snapInfo->snappedToEntity;
            targetPointPos = _registry.get<PositionComponent>(cityEntity).coordinates;
            const auto& cityComp = _registry.get<CityComponent>(cityEntity);
            const auto& activeLine = _registry.ctx().get<ActiveLine>();

            if (!cityComp.connectedLines.empty() && !activeLine.points.empty()) {
                sf::Vector2f incomingDir = targetPointPos - activeLine.points.back().position;
                if (auto len = std::sqrt(incomingDir.x * incomingDir.x + incomingDir.y * incomingDir.y); len > 0) {
                    incomingDir /= len;
                }

                float bestDot = -1.0f;
                for (entt::entity lineEntity : cityComp.connectedLines) {
                    const auto& lineComp = _registry.get<LineComponent>(lineEntity);
                    for (size_t i = 0; i < lineComp.points.size(); ++i) {
                        if (lineComp.points[i].type == LinePointType::STOP && lineComp.points[i].stationEntity == cityEntity) {
                            sf::Vector2f p_prev, p_next;
                            if (i > 0) p_prev = lineComp.points[i - 1].position;
                            else p_prev = lineComp.points[i].position - (lineComp.points[i + 1].position - lineComp.points[i].position);
                            
                            if (i < lineComp.points.size() - 1) p_next = lineComp.points[i + 1].position;
                            else p_next = lineComp.points[i].position + (lineComp.points[i].position - lineComp.points[i - 1].position);

                            sf::Vector2f currentTangent = p_next - p_prev;
                            if (auto len = std::sqrt(currentTangent.x * currentTangent.x + currentTangent.y * currentTangent.y); len > 0) {
                                currentTangent /= len;
                            }

                            float dot = std::abs(incomingDir.x * currentTangent.x + incomingDir.y * currentTangent.y);
                            if (dot > bestDot) {
                                bestDot = dot;
                                tangent = currentTangent;
                                tangentFound = true;
                            }
                            break;
                        }
                    }
                }
            }
            
            if (!tangentFound) {
                const auto& activeLine = _registry.ctx().get<ActiveLine>();
                if (!activeLine.points.empty()) {
                    tangent = targetPointPos - activeLine.points.back().position;
                    tangentFound = true;
                }
            }
        }

        if (tangentFound) {
            if (auto len = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y); len > 0) {
                tangent /= len;
            }
            preview.snapTangent = tangent;

            sf::Vector2f perpendicular = {-tangent.y, tangent.x};
            sf::Vector2f mouseVec = mousePos - targetPointPos;
            float perp_dist = mouseVec.x * perpendicular.x + mouseVec.y * perpendicular.y;

            if (std::abs(perp_dist) < Constants::LINE_CENTER_SNAP_RADIUS) {
                preview.snapSide = 0.f;
                preview.snapPosition = targetPointPos;
            } else {
                preview.snapSide = (perp_dist > 0) ? 1.f : -1.f;
                sf::Vector2f sideOffset = perpendicular * preview.snapSide * Constants::LINE_PARALLEL_OFFSET;
                preview.snapPosition = targetPointPos + sideOffset;
            }
        } else {
            preview.snapPosition = targetPointPos;
        }
        finalPreviewPos = *preview.snapPosition;
    }

    auto& activeLine = _registry.ctx().get<ActiveLine>();
    if (activeLine.points.empty()) {
        return;
    }

    std::vector<sf::Vector2f> previewControlPoints;
    for (const auto& p : activeLine.points) {
        previewControlPoints.push_back(p.position);
    }
    
    previewControlPoints.push_back(finalPreviewPos);

    if (previewControlPoints.size() >= 2) {
        CurveData curveData = Curve::generateMetroCurve(previewControlPoints, Constants::METRO_CURVE_RADIUS);
        preview.curvePoints = curveData.points;

        if (preview.curvePoints.size() >= 2) {
            for (size_t i = 0; i < preview.curvePoints.size() - 1; ++i) {
                const sf::Vector2f& p1 = preview.curvePoints[i];
                const sf::Vector2f& p2 = preview.curvePoints[i+1];
                sf::Vector2f midPoint = p1 + (p2 - p1) * 0.5f;
                
                bool isValid = (_worldGenerationSystem.getTerrainTypeAt(midPoint.x, midPoint.y) != TerrainType::WATER);
                preview.validSegments.push_back(isValid);
            }
        }
    }
}

void LineCreationSystem::onCancelLineCreation(const CancelLineCreationEvent &event) {
    LOG_DEBUG("LineCreationSystem", "Processing CancelLineCreationEvent.");
    clearCurrentLine();
}