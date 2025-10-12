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

    auto &preview = _registry.ctx().get<LinePreview>();
    preview = {}; // Reset the preview

    sf::Vector2f finalPreviewPos = _currentMouseWorldPos;

    if (auto snapResult = SnapHelper::findSnap(_registry, _currentMouseWorldPos)) {
        preview.snapPosition = snapResult->position;
        preview.snapInfo = snapResult->info;
        preview.snapSide = snapResult->side;
        preview.snapTangent = snapResult->tangent;
        finalPreviewPos = snapResult->position;
    }

    auto &activeLine = _registry.ctx().get<ActiveLine>();
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
            for (size_t i = 0; i < preview.curvePoints.size() - 1; ++i) {
                const sf::Vector2f &p1 = preview.curvePoints[i];
                const sf::Vector2f &p2 = preview.curvePoints[i + 1];
                sf::Vector2f midPoint = p1 + (p2 - p1) * 0.5f;
                preview.validSegments.push_back(_worldGenerationSystem.getTerrainTypeAt(midPoint.x, midPoint.y) != TerrainType::WATER);
            }
        }
    }
}

void LineCreationSystem::onCancelLineCreation(const CancelLineCreationEvent &event) {
    LOG_DEBUG("LineCreationSystem", "Processing CancelLineCreationEvent.");
    clearCurrentLine();
}