#include "SharedSegmentSystem.h"
#include "components/LineComponents.h"
#include "Logger.h"
#include <map>

namespace {
    struct Vector2fComparator {
        bool operator()(const sf::Vector2f& a, const sf::Vector2f& b) const {
            if (a.x < b.x) return true;
            if (a.x > b.x) return false;
            return a.y < b.y;
        }
    };

    struct Vector2fPairComparator {
        bool operator()(const std::pair<sf::Vector2f, sf::Vector2f>& a, const std::pair<sf::Vector2f, sf::Vector2f>& b) const {
            Vector2fComparator cmp;
            if (cmp(a.first, b.first)) return true;
            if (cmp(b.first, a.first)) return false;
            return cmp(a.second, b.second);
        }
    };

    sf::Vector2f quantize(const sf::Vector2f& vec) {
        const float precision = 100.0f; // Adjust precision as needed
        return {std::round(vec.x * precision) / precision, std::round(vec.y * precision) / precision};
    }
}

SharedSegmentSystem::SharedSegmentSystem(entt::registry& registry, EventBus& eventBus)
    : _registry(registry), _eventBus(eventBus) {
    _lineModifiedConnection = _eventBus.sink<LineModifiedEvent>().connect<&SharedSegmentSystem::onLineModified>(this);
    processSharedSegments(); // Initial processing
}

SharedSegmentSystem::~SharedSegmentSystem() {
    _lineModifiedConnection.release();
}

void SharedSegmentSystem::onLineModified(const LineModifiedEvent& event) {
    LOG_INFO("SharedSegmentSystem", "Line %u modified, reprocessing shared segments.", entt::to_integral(event.lineEntity));
    processSharedSegments();
}

void SharedSegmentSystem::processSharedSegments() {
    // Clear old data
    if (_registry.ctx().contains<SharedSegmentsContext>()) {
        _registry.ctx().erase<SharedSegmentsContext>();
    }
    _registry.ctx().emplace<SharedSegmentsContext>();
    auto& sharedSegmentsCtx = _registry.ctx().get<SharedSegmentsContext>();

    auto lineViewClear = _registry.view<LineComponent>();
    for (auto entity : lineViewClear) {
        auto& line = lineViewClear.get<LineComponent>(entity);
        line.sharedSegments.clear();
    }

    // Build segment map
    std::map<std::pair<sf::Vector2f, sf::Vector2f>, std::vector<std::pair<entt::entity, size_t>>, Vector2fPairComparator> segmentMap;

    auto lineViewBuild = _registry.view<LineComponent>();
    for (auto entity : lineViewBuild) {
        auto& line = lineViewBuild.get<LineComponent>(entity);
        if (line.points.size() < 2) continue;

        for (size_t i = 0; i < line.points.size() - 1; ++i) {
            sf::Vector2f p1 = quantize(line.points[i].position); // Quantize here
            sf::Vector2f p2 = quantize(line.points[i+1].position); // And here

            // Canonical ordering
            if (p1.x > p2.x || (p1.x == p2.x && p1.y > p2.y)) {
                std::swap(p1, p2);
            }
            
            segmentMap[{p1, p2}].push_back({entity, i});
        }
    }

    // Process shared segments
    for (auto const& [segmentPoints, lineInfos] : segmentMap) {
        if (lineInfos.size() > 1) {
            SharedSegment newSharedSegment;
            for (const auto& lineInfo : lineInfos) {
                newSharedSegment.lines.push_back(lineInfo.first);
            }
            
            // Sort lines for deterministic ordering
            std::sort(newSharedSegment.lines.begin(), newSharedSegment.lines.end());

            auto& sharedSegment = sharedSegmentsCtx.segments[segmentPoints] = newSharedSegment;

            for (const auto& lineInfo : lineInfos) {
                auto& line = _registry.get<LineComponent>(lineInfo.first);
                line.sharedSegments[lineInfo.second] = &sharedSegment;
            }
        }
    }
    LOG_INFO("SharedSegmentSystem", "Finished processing shared segments. Found %d shared segments.", sharedSegmentsCtx.segments.size());
}