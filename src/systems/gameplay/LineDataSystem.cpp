#include "LineDataSystem.h"
#include "components/GameLogicComponents.h"
#include "ecs/EntityFactory.h"
#include "Logger.h"
#include <SFML/System/Vector2.hpp>
#include <vector>
#include <map>
#include <cmath>

// A custom comparator for sf::Vector2f so it can be used as a map key.
struct Vector2fComparator {
    bool operator()(const sf::Vector2f& a, const sf::Vector2f& b) const {
        if (a.x < b.x) return true;
        if (a.x > b.x) return false;
        return a.y < b.y;
    }
};

LineDataSystem::LineDataSystem(entt::registry& registry, EntityFactory& entityFactory, EventBus& eventBus)
    : _registry(registry),
      _entityFactory(entityFactory) {
    LOG_DEBUG("LineDataSystem", "LineDataSystem created.");
    m_addTrainConnection = eventBus.sink<AddTrainToLineEvent>().connect<&LineDataSystem::onAddTrain>(this);
}

LineDataSystem::~LineDataSystem() {
    m_addTrainConnection.release();
}

void LineDataSystem::update(sf::Time dt) {
    processParallelSegments();
}

void LineDataSystem::onAddTrain(const AddTrainToLineEvent& event) {
    LOG_DEBUG("LineDataSystem", "Processing AddTrainToLineEvent for line %u.", entt::to_integral(event.lineEntity));
    _entityFactory.createTrain(event.lineEntity);
}

void LineDataSystem::processParallelSegments() {
    std::map<std::pair<entt::entity, entt::entity>, std::vector<std::pair<entt::entity, size_t>>> segmentMap;

    auto lineView = _registry.view<LineComponent>();
    for (auto entity : lineView) {
        auto &line = lineView.get<LineComponent>(entity);
        if (line.points.size() < 2) {
            line.pathOffsets.clear();
            continue;
        }

        line.pathOffsets.assign(line.points.size() - 1, {0.f, 0.f});

        for (size_t i = 0; i < line.points.size() - 1; ++i) {
            if (line.points[i].type == LinePointType::STOP && line.points[i+1].type == LinePointType::STOP) {
                entt::entity station1 = line.points[i].stationEntity;
                entt::entity station2 = line.points[i+1].stationEntity;

                if (station1 > station2) {
                    std::swap(station1, station2);
                }
                segmentMap[{station1, station2}].push_back({entity, i});
            }
        }
    }

    const float offsetStep = 12.0f;

    for (const auto& [segment, lines] : segmentMap) {
        if (lines.size() > 1) {
            std::vector<std::pair<entt::entity, size_t>> forwardLines;
            std::vector<std::pair<entt::entity, size_t>> reverseLines;

            // Separate lines by direction relative to the canonical segment
            for (const auto& lineInfo : lines) {
                auto& line = _registry.get<LineComponent>(lineInfo.first);
                if (line.points[lineInfo.second].stationEntity == segment.first) {
                    reverseLines.push_back(lineInfo);
                } else {
                    forwardLines.push_back(lineInfo);
                }
            }

            const auto& pos1 = _registry.get<PositionComponent>(segment.first).coordinates;
            const auto& pos2 = _registry.get<PositionComponent>(segment.second).coordinates;
            sf::Vector2f canonicalDirection = pos2 - pos1;
            float length = std::sqrt(canonicalDirection.x * canonicalDirection.x + canonicalDirection.y * canonicalDirection.y);
            if (length == 0) continue;
            sf::Vector2f canonicalPerpendicular(-canonicalDirection.y / length, canonicalDirection.x / length);

            // Process forward lines
            std::sort(forwardLines.begin(), forwardLines.end());
            float currentOffset = offsetStep / 2.0f;
            for (const auto& lineInfo : forwardLines) {
                auto& line = _registry.get<LineComponent>(lineInfo.first);
                line.pathOffsets[lineInfo.second] = canonicalPerpendicular * currentOffset;
                currentOffset += offsetStep;
            }

            // Process reverse lines
            std::sort(reverseLines.begin(), reverseLines.end());
            currentOffset = offsetStep / 2.0f;
            for (const auto& lineInfo : reverseLines) {
                auto& line = _registry.get<LineComponent>(lineInfo.first);
                line.pathOffsets[lineInfo.second] = -canonicalPerpendicular * currentOffset;
                currentOffset += offsetStep;
            }
        }
    }
}