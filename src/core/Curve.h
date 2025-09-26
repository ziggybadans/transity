#pragma once

#include <SFML/System/Vector2.hpp>
#include <vector>
#include <cmath>
#include "components/GameLogicComponents.h"

struct CurveData {
    std::vector<sf::Vector2f> points;
    std::vector<size_t> segmentIndices;
};

class Curve {
public:
    static CurveData generateCatmullRom(const std::vector<sf::Vector2f>& points, int pointsPerSegment = 10) {
        CurveData curveData;
        if (points.size() < 2) {
            curveData.points = points;
            if (!points.empty()) {
                curveData.segmentIndices.push_back(0);
            }
            return curveData;
        }

        curveData.points.push_back(points[0]);
        curveData.segmentIndices.push_back(0);

        for (size_t i = 0; i < points.size() - 1; ++i) {
            const sf::Vector2f& p0 = (i == 0) ? points[i] : points[i - 1];
            const sf::Vector2f& p1 = points[i];
            const sf::Vector2f& p2 = points[i + 1];
            const sf::Vector2f& p3 = (i + 2 < points.size()) ? points[i + 2] : p2;

            for (int j = 1; j <= pointsPerSegment; ++j) {
                float t = static_cast<float>(j) / pointsPerSegment;
                sf::Vector2f point = 0.5f * (
                    (2.f * p1) +
                    (-p0 + p2) * t +
                    (2.f * p0 - 5.f * p1 + 4.f * p2 - p3) * t * t +
                    (-p0 + 3.f * p1 - 3.f * p2 + p3) * t * t * t
                );
                curveData.points.push_back(point);
                curveData.segmentIndices.push_back(i);
            }
        }

        return curveData;
    }

    static float calculateCurveLength(const std::vector<sf::Vector2f>& curvePoints) {
        float length = 0.0f;
        for (size_t i = 0; i < curvePoints.size() - 1; ++i) {
            const sf::Vector2f& p1 = curvePoints[i];
            const sf::Vector2f& p2 = curvePoints[i + 1];
            length += std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
        }
        return length;
    }

    static std::vector<StopInfo> calculateStopInfo(const std::vector<LinePoint>& linePoints, const std::vector<sf::Vector2f>& curvePoints) {
        std::vector<StopInfo> stopInfo;
        if (linePoints.empty() || curvePoints.empty()) {
            return stopInfo;
        }

        float currentCurveDistance = 0.0f;
        size_t nextPointIndex = 0;
        for(size_t i = 0; i < curvePoints.size(); ++i) {
            if (nextPointIndex < linePoints.size()) {
                const auto& p = linePoints[nextPointIndex];
                if (curvePoints[i] == p.position) {
                    if (p.type == LinePointType::STOP) {
                        stopInfo.push_back({p.stationEntity, currentCurveDistance});
                    }
                    nextPointIndex++;
                }
            }
            if (i < curvePoints.size() - 1) {
                const auto& p1 = curvePoints[i];
                const auto& p2 = curvePoints[i + 1];
                currentCurveDistance += std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
            }
        }
        return stopInfo;
    }
};