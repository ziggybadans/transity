#include "LineCreationSystem.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "ecs/EntityFactory.h"
#include "render/ColorManager.h"
#include "imgui.h"
#include "Constants.h"
#include "core/Curve.h"
#include <algorithm>
#include <utility>

LineCreationSystem::LineCreationSystem(entt::registry& registry, EntityFactory& entityFactory, ColorManager& colorManager, GameState& gameState, EventBus& eventBus)
    : _registry(registry), _entityFactory(entityFactory),
      _colorManager(colorManager), _gameState(gameState), _eventBus(eventBus) {

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

    sf::Color chosenColor = _colorManager.getNextLineColor();
    entt::entity lineEntity = _entityFactory.createLine(activeLine.points, chosenColor);

    if (lineEntity == entt::null) {
        LOG_ERROR("LineCreationSystem", "Failed to create line entity.");
        clearCurrentLine();
        return;
    }

    auto& newLineComp = _registry.get<LineComponent>(lineEntity);
    for (size_t i = 0; i < activeLine.points.size() - 1; ++i) {
        const auto& p1 = activeLine.points[i];
        const auto& p2 = activeLine.points[i+1];

        if (p1.snapInfo && p2.snapInfo && p1.snapSide != 0.f && p1.snapSide == p2.snapSide) {
            sf::Vector2f original_p1_pos, original_p2_pos;

            if (p1.snapInfo->snappedToPointIndex != (size_t)-1) {
                const auto& line = _registry.get<LineComponent>(p1.snapInfo->snappedToEntity);
                original_p1_pos = line.points[p1.snapInfo->snappedToPointIndex].position;
            } else {
                original_p1_pos = _registry.get<PositionComponent>(p1.snapInfo->snappedToEntity).coordinates;
            }

            if (p2.snapInfo->snappedToPointIndex != (size_t)-1) {
                const auto& line = _registry.get<LineComponent>(p2.snapInfo->snappedToEntity);
                original_p2_pos = line.points[p2.snapInfo->snappedToPointIndex].position;
            } else {
                original_p2_pos = _registry.get<PositionComponent>(p2.snapInfo->snappedToEntity).coordinates;
            }

            sf::Vector2f tangent = original_p2_pos - original_p1_pos;
            if (auto len = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y); len > 0) {
                tangent /= len;
            } else {
                continue;
            }

            sf::Vector2f perpendicular = {-tangent.y, tangent.x};
            
            if (i < newLineComp.pathOffsets.size()) {
                newLineComp.pathOffsets[i] = perpendicular * p1.snapSide * Constants::LINE_PARALLEL_OFFSET;
            }
        }
    }

    if (newLineComp.points.size() > 1) {
        if (newLineComp.pathOffsets.size() > 0 && newLineComp.pathOffsets[0] != sf::Vector2f(0,0)) {
            newLineComp.points[0].position += newLineComp.pathOffsets[0];
        }
        size_t lastSegmentIndex = newLineComp.curveSegmentIndices.back();
        if (newLineComp.pathOffsets.size() > lastSegmentIndex && newLineComp.pathOffsets[lastSegmentIndex] != sf::Vector2f(0,0)) {
            newLineComp.points.back().position += newLineComp.pathOffsets[lastSegmentIndex];
        }
    }
    
    std::vector<sf::Vector2f> controlPoints;
    for (const auto &point : newLineComp.points) {
        controlPoints.push_back(point.position);
    }
    CurveData curveData = Curve::generateCatmullRom(controlPoints);
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
                            if (i > 0) {
                                p_prev = lineComp.points[i - 1].position;
                            } else {
                                p_prev = lineComp.points[i].position - (lineComp.points[i + 1].position - lineComp.points[i].position);
                            }
                            if (i < lineComp.points.size() - 1) {
                                p_next = lineComp.points[i + 1].position;
                            } else {
                                p_next = lineComp.points[i].position + (lineComp.points[i].position - lineComp.points[i - 1].position);
                            }

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
                } else {
                    preview.snapPosition = targetPointPos;
                    return;
                }
            }
        }

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
    }
}

void LineCreationSystem::onCancelLineCreation(const CancelLineCreationEvent &event) {
    LOG_DEBUG("LineCreationSystem", "Processing CancelLineCreationEvent.");
    clearCurrentLine();
}