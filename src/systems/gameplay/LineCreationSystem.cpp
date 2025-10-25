#include "LineCreationSystem.h"
#include "Logger.h"
#include "components/LineComponents.h"
#include "SnapHelper.h"
#include "systems/world/WorldGenerationSystem.h"
#include "ecs/EntityFactory.h"
#include "render/ColorManager.h"
#include "imgui.h"
#include "Constants.h"
#include "core/Curve.h"
#include "event/LineEvents.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <queue>
#include <random>
#include <vector>

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

    if (_gameState.currentInteractionMode != InteractionMode::CREATE_LINE) {
        return;
    }

    if (event.button == sf::Mouse::Button::Right) {
        if (!_registry.ctx().contains<ActiveLine>()) {
            _registry.ctx().emplace<ActiveLine>();
        }

        auto &activeLine = _registry.ctx().get<ActiveLine>();
        if (activeLine.points.empty()) {
            return;
        }

        const LinePoint removedPoint = activeLine.points.back();
        activeLine.points.pop_back();
        commitActiveLineCurve(activeLine);

        if (removedPoint.type == LinePointType::STOP) {
            LOG_DEBUG("LineCreationSystem", "Removed stop point from active line.");
        } else {
            LOG_DEBUG("LineCreationSystem", "Removed control point from active line.");
        }
        return;
    }

    if (event.button == sf::Mouse::Button::Left) {
        auto& preview = _registry.ctx().get<LinePreview>();
        
        if (!preview.validSegments.empty() && preview.curvePoints.size() >= 2) {
            const bool hasInvalidSegment = std::any_of(preview.validSegments.begin(), preview.validSegments.end(),
                                                       [](bool value) { return !value; });
            if (hasInvalidSegment) {
                if (preview.currentSegmentCrossesWater) {
                    LOG_WARN("LineCreationSystem", "Cannot place point: the path crosses water.");
                } else if (preview.currentSegmentExceedsGrade) {
                    LOG_WARN("LineCreationSystem", "Cannot place point: the grade exceeds %.1f%%.",
                             MAX_ALLOWED_GRADE * 100.0f);
                } else {
                    LOG_WARN("LineCreationSystem", "Cannot place point: the segment is invalid.");
                }
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

    auto &activeLine = _registry.ctx().get<ActiveLine>();

    if (activeLine.points.empty() && stationEntity == entt::null) {
        LOG_WARN("LineCreationSystem", "The first point of a line must be a station.");
        return;
    }

    const bool addingStop = (stationEntity != entt::null);
    if (addingStop && !activeLine.points.empty()) {
        const auto &lastPoint = activeLine.points.back();
        if (lastPoint.type == LinePointType::STOP && lastPoint.stationEntity == stationEntity) {
            LOG_WARN("LineCreationSystem",
                     "Station %u is already the last point in the active line.",
                     static_cast<unsigned int>(stationEntity));
            return;
        }
    }

    LinePoint newPoint;
    if (addingStop) {
        newPoint = {LinePointType::STOP, position, stationEntity, snapInfo, snapSide};
    } else {
        newPoint = {LinePointType::CONTROL_POINT, position, entt::null, snapInfo, snapSide};
    }

    activeLine.points.push_back(newPoint);

    if (!commitActiveLineCurve(activeLine)) {
        activeLine.points.pop_back();
        return;
    }

    if (addingStop) {
        LOG_DEBUG("LineCreationSystem", "Station %u added to active line.",
                  static_cast<unsigned int>(stationEntity));
    } else {
        LOG_DEBUG("LineCreationSystem", "Control point added to active line at (%.1f, %.1f).",
                  position.x, position.y);
    }
}

LineCreationSystem::SegmentValidationResult
LineCreationSystem::validateStraightSegment(const sf::Vector2f &from, const sf::Vector2f &to) const {
    SegmentValidationResult result;
    const bool enforceGradeLimit = _gameState.elevationChecksEnabled;

    std::array<sf::Vector2f, SEGMENT_INTERIOR_SAMPLES + 2> samples{};
    samples[0] = from;
    samples[SEGMENT_INTERIOR_SAMPLES + 1] = to;
    for (int i = 1; i <= SEGMENT_INTERIOR_SAMPLES; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SEGMENT_INTERIOR_SAMPLES + 1);
        samples[i] = from + (to - from) * t;
    }

    for (int i = 1; i <= SEGMENT_INTERIOR_SAMPLES; ++i) {
        const auto &point = samples[i];
        if (_worldGenerationSystem.getTerrainTypeAt(point.x, point.y) == TerrainType::WATER) {
            result.crossesWater = true;
            break;
        }
    }

    const float epsilon = std::numeric_limits<float>::epsilon();
    for (int i = 0; i <= SEGMENT_INTERIOR_SAMPLES; ++i) {
        const auto &start = samples[i];
        const auto &end = samples[i + 1];
        const sf::Vector2f delta = end - start;
        float distance = std::hypot(delta.x, delta.y);
        if (distance <= epsilon) {
            continue;
        }
        float elevationStart = _worldGenerationSystem.getElevationAt(start.x, start.y);
        float elevationEnd = _worldGenerationSystem.getElevationAt(end.x, end.y);
        float grade = std::abs(elevationEnd - elevationStart) / distance;
        if (grade > result.maxGrade) {
            result.maxGrade = grade;
        }
        if (grade > MAX_ALLOWED_GRADE) {
            result.exceedsGrade = true;
            if (enforceGradeLimit) {
                break;
            }
        }
    }

    const bool gradeViolation = enforceGradeLimit && result.exceedsGrade;
    result.isValid = !result.crossesWater && !gradeViolation;
    return result;
}

bool LineCreationSystem::commitActiveLineCurve(ActiveLine &activeLine) {
    if (activeLine.points.empty()) {
        activeLine.committedCurvePoints.clear();
        return true;
    }

    if (activeLine.points.size() >= 2) {
        const sf::Vector2f &from = activeLine.points[activeLine.points.size() - 2].position;
        const sf::Vector2f &to = activeLine.points.back().position;
        SegmentValidationResult validation = validateStraightSegment(from, to);
        if (!validation.isValid) {
            if (validation.crossesWater) {
                LOG_WARN("LineCreationSystem", "Cannot place point: resulting path would cross water.");
            } else if (validation.exceedsGrade) {
                LOG_WARN("LineCreationSystem",
                         "Cannot place point: resulting path would exceed maximum grade of %.1f%%.",
                         MAX_ALLOWED_GRADE * 100.0f);
            } else {
                LOG_WARN("LineCreationSystem", "Cannot place point: resulting path is invalid.");
            }
            return false;
        }
    }

    std::vector<sf::Vector2f> controlPoints;
    controlPoints.reserve(activeLine.points.size());
    for (const auto &point : activeLine.points) {
        controlPoints.push_back(point.position);
    }

    CurveData curveData = Curve::generateMetroCurve(controlPoints, Constants::METRO_CURVE_RADIUS);
    activeLine.committedCurvePoints = std::move(curveData.points);
    return true;
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

    for (size_t i = 0; i + 1 < activeLine.points.size(); ++i) {
        const sf::Vector2f &from = activeLine.points[i].position;
        const sf::Vector2f &to = activeLine.points[i + 1].position;
        SegmentValidationResult validation = validateStraightSegment(from, to);
        if (!validation.isValid) {
            if (validation.crossesWater) {
                LOG_WARN("LineCreationSystem", "Cannot finalize line: path intersects with water.");
            } else if (validation.exceedsGrade) {
                LOG_WARN("LineCreationSystem",
                         "Cannot finalize line: path exceeds maximum grade of %.1f%%.",
                         MAX_ALLOWED_GRADE * 100.0f);
            } else {
                LOG_WARN("LineCreationSystem", "Cannot finalize line: path is invalid.");
            }
            return;
        }
    }

    std::vector<sf::Vector2f> controlPointsForFinalize;
    controlPointsForFinalize.reserve(activeLine.points.size());
    for (const auto &point : activeLine.points) {
        controlPointsForFinalize.push_back(point.position);
    }
    CurveData finalCurveData = Curve::generateMetroCurve(controlPointsForFinalize, Constants::METRO_CURVE_RADIUS);

    sf::Color chosenColor = _colorManager.getNextLineColor();
    entt::entity lineEntity = _entityFactory.createLine(activeLine.points, chosenColor);

    if (lineEntity == entt::null) {
        LOG_ERROR("LineCreationSystem", "Failed to create line entity.");
        clearCurrentLine();
        return;
    }

    auto& newLineComp = _registry.get<LineComponent>(lineEntity);
    
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

    auto &preview = _registry.ctx().get<LinePreview>();
    preview = {}; // Reset the preview

    sf::Vector2f finalPreviewPos = _currentMouseWorldPos;

    auto &activeLine = _registry.ctx().get<ActiveLine>();
    std::optional<sf::Vector2f> prevPointPos;
    if (!activeLine.points.empty()) {
        prevPointPos = activeLine.points.back().position;
    }

    if (auto snapResult = SnapHelper::findSnap(_registry, _currentMouseWorldPos, prevPointPos)) {
        preview.snapPosition = snapResult->position;
        preview.snapInfo = snapResult->info;
        preview.snapSide = snapResult->side;
        preview.snapTangent = snapResult->tangent;
        finalPreviewPos = snapResult->position;
    }

    if (activeLine.points.empty()) {
        return;
    }

    std::vector<sf::Vector2f> previewControlPoints;
    for (const auto &p : activeLine.points) {
        previewControlPoints.push_back(p.position);
    }
    previewControlPoints.push_back(finalPreviewPos);

    if (previewControlPoints.size() >= 2) {
        CurveData curveData = Curve::generateMetroCurve(previewControlPoints, Constants::METRO_CURVE_RADIUS);
        preview.curvePoints = curveData.points;

        if (preview.curvePoints.size() >= 2) {
            std::size_t lockedSegments = 0;
            if (activeLine.committedCurvePoints.size() >= 2) {
                lockedSegments = activeLine.committedCurvePoints.size() - 1;
            }

            const std::size_t totalSegments = preview.curvePoints.size() - 1;
            if (lockedSegments > totalSegments) {
                lockedSegments = totalSegments;
            }

            preview.validSegments.assign(totalSegments, true);
            preview.currentSegmentGrade.reset();
            preview.currentSegmentExceedsGrade = false;
            preview.currentSegmentCrossesWater = false;

            if (totalSegments > lockedSegments) {
                const sf::Vector2f &from = activeLine.points.back().position;
                const sf::Vector2f &to = finalPreviewPos;
                SegmentValidationResult validation = validateStraightSegment(from, to);

                for (std::size_t i = lockedSegments; i < totalSegments; ++i) {
                    preview.validSegments[i] = validation.isValid;
                }

                preview.currentSegmentGrade = validation.maxGrade;
                preview.currentSegmentExceedsGrade = validation.exceedsGrade;
                preview.currentSegmentCrossesWater = validation.crossesWater;
            }
        }
    }
}

void LineCreationSystem::onCancelLineCreation(const CancelLineCreationEvent &event) {
    LOG_DEBUG("LineCreationSystem", "Processing CancelLineCreationEvent.");
    clearCurrentLine();
}
