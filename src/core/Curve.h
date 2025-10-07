#pragma once

#include "components/GameLogicComponents.h"
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <vector>
#include <algorithm>
#include <limits>

struct CurveData {
    std::vector<sf::Vector2f> points;
    std::vector<size_t> segmentIndices;
};

class Curve {
public:
    static CurveData generateMetroCurve(const std::vector<sf::Vector2f>& points, float radius, int pointsPerArc = 10) {
        CurveData curveData;
        if (points.size() < 2) {
            if (!points.empty()) {
                curveData.points = {points[0]};
                curveData.segmentIndices = {0};
            }
            return curveData;
        }

        if (points.size() < 3) {
            curveData.points = points;
            for (size_t i = 0; i < points.size() - 1; ++i) {
                curveData.segmentIndices.push_back(i);
            }
            if (!points.empty()) {
                curveData.segmentIndices.push_back(points.size() - 2);
            }
            return curveData;
        }

        struct Arc {
            sf::Vector2f start;
            sf::Vector2f corner;
            sf::Vector2f end;
        };
        std::vector<Arc> arcs;

        for (size_t i = 1; i < points.size() - 1; ++i) {
            const sf::Vector2f& p_prev = points[i - 1];
            const sf::Vector2f& p_curr = points[i];
            const sf::Vector2f& p_next = points[i + 1];

            sf::Vector2f v1 = p_prev - p_curr;
            sf::Vector2f v2 = p_next - p_curr;

            float len1 = std::sqrt(v1.x * v1.x + v1.y * v1.y);
            float len2 = std::sqrt(v2.x * v2.x + v2.y * v2.y);

            float currentRadius = radius;
            if (len1 > 0 && currentRadius > len1 / 2.0f) currentRadius = len1 / 2.0f;
            if (len2 > 0 && currentRadius > len2 / 2.0f) currentRadius = len2 / 2.0f;

            sf::Vector2f arc_start = (len1 > 0) ? p_curr + (v1 / len1) * currentRadius : p_curr;
            sf::Vector2f arc_end = (len2 > 0) ? p_curr + (v2 / len2) * currentRadius : p_curr;
            
            arcs.push_back({arc_start, p_curr, arc_end});
        }

        curveData.points.push_back(points[0]);
        curveData.segmentIndices.push_back(0);

        for (size_t i = 0; i < points.size() - 1; ++i) {
            sf::Vector2f end_of_segment = (i == points.size() - 2) ? points.back() : arcs[i].start;

            if (curveData.points.back() != end_of_segment) {
                curveData.points.push_back(end_of_segment);
                curveData.segmentIndices.push_back(i);
            }

            if (i < arcs.size()) {
                const auto& arc = arcs[i];
                for (int j = 1; j <= pointsPerArc; ++j) {
                    float t = static_cast<float>(j) / pointsPerArc;
                    sf::Vector2f point = (1.0f - t) * (1.0f - t) * arc.start + 2.0f * (1.0f - t) * t * arc.corner + t * t * arc.end;
                    curveData.points.push_back(point);
                    curveData.segmentIndices.push_back(i);
                }
            }
        }

        return curveData;
    }

    static CurveData generateCatmullRom(const std::vector<sf::Vector2f> &points,
                                        int pointsPerSegment = 10) {
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
            const sf::Vector2f &p0 = (i == 0) ? points[i] : points[i - 1];
            const sf::Vector2f &p1 = points[i];
            const sf::Vector2f &p2 = points[i + 1];
            const sf::Vector2f &p3 = (i + 2 < points.size()) ? points[i + 2] : p2;

            for (int j = 1; j <= pointsPerSegment; ++j) {
                float t = static_cast<float>(j) / pointsPerSegment;
                sf::Vector2f point =
                    0.5f
                    * ((2.f * p1) + (-p0 + p2) * t + (2.f * p0 - 5.f * p1 + 4.f * p2 - p3) * t * t
                       + (-p0 + 3.f * p1 - 3.f * p2 + p3) * t * t * t);
                curveData.points.push_back(point);
                curveData.segmentIndices.push_back(i);
            }
        }

        return curveData;
    }

    static float calculateCurveLength(const std::vector<sf::Vector2f> &curvePoints) {
        float length = 0.0f;
        for (size_t i = 0; i < curvePoints.size() - 1; ++i) {
            const sf::Vector2f &p1 = curvePoints[i];
            const sf::Vector2f &p2 = curvePoints[i + 1];
            length += std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
        }
        return length;
    }

    static std::vector<StopInfo> calculateStopInfo(const std::vector<LinePoint> &linePoints,
                                               const std::vector<sf::Vector2f> &curvePoints) {
    std::vector<StopInfo> stopInfo;
        if (linePoints.empty() || curvePoints.empty()) {
            return stopInfo;
        }

        std::vector<float> cumulativeDistances;
        cumulativeDistances.push_back(0.f);
        float totalLength = 0.f;

        for (size_t i = 0; i < curvePoints.size() - 1; ++i) {
            const auto &p1 = curvePoints[i];
            const auto &p2 = curvePoints[i + 1];
            float segmentLength = std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
            totalLength += segmentLength;
            cumulativeDistances.push_back(totalLength);
        }

        for (const auto& linePoint : linePoints) {
            if (linePoint.type == LinePointType::STOP) {
                float min_dist_sq = std::numeric_limits<float>::max();
                size_t closest_point_index = 0;

                for (size_t i = 0; i < curvePoints.size(); ++i) {
                    float dx = curvePoints[i].x - linePoint.position.x;
                    float dy = curvePoints[i].y - linePoint.position.y;
                    float dist_sq = dx * dx + dy * dy;
                    if (dist_sq < min_dist_sq) {
                        min_dist_sq = dist_sq;
                        closest_point_index = i;
                    }
                }
                
                if (closest_point_index < cumulativeDistances.size()) {
                    stopInfo.push_back({linePoint.stationEntity, cumulativeDistances[closest_point_index]});
                }
            }
        }
        
        std::sort(stopInfo.begin(), stopInfo.end(), [](const StopInfo& a, const StopInfo& b) {
            return a.distanceAlongCurve < b.distanceAlongCurve;
        });

        return stopInfo;
    }
};