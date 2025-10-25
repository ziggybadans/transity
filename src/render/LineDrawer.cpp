#include "render/LineDrawer.h"
#include <algorithm>
#include <cmath>

void LineDrawer::createThickLine(sf::VertexArray &vertices, const std::vector<sf::Vector2f> &points,
                                 float thickness, sf::Color color) {
    if (points.size() < 2) {
        vertices.clear();
        return;
    }

    vertices.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
    vertices.resize(points.size() * 2);

    float halfThickness = thickness / 2.f;

    for (std::size_t i = 0; i < points.size(); ++i) {
        sf::Vector2f p1 = points[i];
        sf::Vector2f p0 = (i > 0) ? points[i - 1] : p1;
        sf::Vector2f p2 = (i < points.size() - 1) ? points[i + 1] : p1;

        sf::Vector2f dir1 = p1 - p0;
        float len1 = std::sqrt(dir1.x * dir1.x + dir1.y * dir1.y);
        if (len1 != 0) dir1 /= len1;

        sf::Vector2f dir2 = p2 - p1;
        float len2 = std::sqrt(dir2.x * dir2.x + dir2.y * dir2.y);
        if (len2 != 0) dir2 /= len2;

        sf::Vector2f normal;
        if (i == 0) {
            normal = sf::Vector2f(-dir2.y, dir2.x);
        } else if (i == points.size() - 1) {
            normal = sf::Vector2f(-dir1.y, dir1.x);
        } else {
            sf::Vector2f miter = dir1 + dir2;
            float miterLen = std::sqrt(miter.x * miter.x + miter.y * miter.y);
            if (miterLen > 1e-6) {
                normal = sf::Vector2f(-miter.y, miter.x) / miterLen;
            } else {
                normal = sf::Vector2f(-dir1.y, dir1.x);
            }
        }

        float miterRatio = 1.0f;
        if (i > 0 && i < points.size() - 1) {
            float dot = dir1.x * dir2.x + dir1.y * dir2.y;
            dot = std::max(-0.99f, std::min(0.99f, dot));
            miterRatio = std::sqrt(2.0f / (1.0f + dot));
        }

        if (miterRatio > 2.5f) miterRatio = 2.5f;

        vertices[i * 2].position = p1 - normal * (halfThickness * miterRatio);
        vertices[i * 2].color = color;
        vertices[i * 2 + 1].position = p1 + normal * (halfThickness * miterRatio);
        vertices[i * 2 + 1].color = color;
    }
}

void LineDrawer::drawBarberPolePolyline(sf::RenderTarget &target,
                                        const std::vector<sf::Vector2f> &points, float thickness,
                                        const std::vector<sf::Color> &colors, float phaseOffset) {
    if (points.size() < 2 || colors.empty()) return;

    std::vector<sf::Vector2f> miterNormals(points.size());
    std::vector<float> miterRatios(points.size(), 1.0f);
    float halfThickness = thickness / 2.f;

    for (size_t i = 0; i < points.size(); ++i) {
        sf::Vector2f p1 = points[i];
        sf::Vector2f p0 = (i > 0) ? points[i - 1] : p1;
        sf::Vector2f p2 = (i < points.size() - 1) ? points[i + 1] : p1;

        sf::Vector2f dir1 = p1 - p0;
        float len1 = std::sqrt(dir1.x * dir1.x + dir1.y * dir1.y);
        if (len1 != 0) dir1 /= len1;

        sf::Vector2f dir2 = p2 - p1;
        float len2 = std::sqrt(dir2.x * dir2.x + dir2.y * dir2.y);
        if (len2 != 0) dir2 /= len2;

        if (i == 0) {
            miterNormals[i] = sf::Vector2f(-dir2.y, dir2.x);
        } else if (i == points.size() - 1) {
            miterNormals[i] = sf::Vector2f(-dir1.y, dir1.x);
        } else {
            sf::Vector2f miter = dir1 + dir2;
            float miterLen = std::sqrt(miter.x * miter.x + miter.y * miter.y);
            if (miterLen > 1e-6) {
                miterNormals[i] = sf::Vector2f(-miter.y, miter.x) / miterLen;
            } else {
                miterNormals[i] = sf::Vector2f(-dir1.y, dir1.x);
            }

            float dot = dir1.x * dir2.x + dir1.y * dir2.y;
            dot = std::max(-0.99f, std::min(0.99f, dot));
            miterRatios[i] = std::min(2.5f, std::sqrt(2.0f / (1.0f + dot)));
        }
    }

    float totalLength = 0.f;
    std::vector<float> segmentLengths;
    for (size_t i = 0; i < points.size() - 1; ++i) {
        sf::Vector2f dir = points[i + 1] - points[i];
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        segmentLengths.push_back(len);
        totalLength += len;
    }

    if (totalLength == 0.f) return;

    const float stripeLength = 10.0f;
    const float animationOffset = phaseOffset;

    float distanceAlongPath = 0.f;
    for (size_t i = 0; i < points.size() - 1; ++i) {
        const sf::Vector2f &p1 = points[i];
        const sf::Vector2f &p2 = points[i + 1];
        float segmentLen = segmentLengths[i];
        if (segmentLen == 0) continue;

        sf::Vector2f dir = (p2 - p1) / segmentLen;

        sf::Vector2f thicknessOffset1 = miterNormals[i] * (halfThickness * miterRatios[i]);
        sf::Vector2f thicknessOffset2 = miterNormals[i + 1] * (halfThickness * miterRatios[i + 1]);

        float startDist = distanceAlongPath;
        float endDist = startDist + segmentLen;

        float currentStripeStart =
            std::ceil((startDist - animationOffset) / stripeLength) * stripeLength
            + animationOffset;

        while (currentStripeStart < endDist) {
            float stripeEnd = currentStripeStart + stripeLength;

            int colorIndex =
                static_cast<int>((currentStripeStart - animationOffset) / stripeLength);
            sf::Color color = colors[std::abs(colorIndex) % colors.size()];

            float clampedStripeStartDist = std::max(startDist, currentStripeStart);
            float clampedStripeEndDist = std::min(endDist, stripeEnd);

            if (clampedStripeStartDist < clampedStripeEndDist) {
                float t1 = (clampedStripeStartDist - startDist) / segmentLen;
                float t2 = (clampedStripeEndDist - startDist) / segmentLen;

                sf::Vector2f stripe_p1 = p1 + dir * (clampedStripeStartDist - startDist);
                sf::Vector2f stripe_p2 = p1 + dir * (clampedStripeEndDist - startDist);

                sf::Vector2f offset1 = thicknessOffset1 * (1.f - t1) + thicknessOffset2 * t1;
                sf::Vector2f offset2 = thicknessOffset1 * (1.f - t2) + thicknessOffset2 * t2;

                sf::VertexArray stripe(sf::PrimitiveType::TriangleStrip, 4);
                stripe[0].position = stripe_p1 - offset1;
                stripe[1].position = stripe_p1 + offset1;
                stripe[2].position = stripe_p2 - offset2;
                stripe[3].position = stripe_p2 + offset2;

                stripe[0].color = color;
                stripe[1].color = color;
                stripe[2].color = color;
                stripe[3].color = color;

                target.draw(stripe);
            }
            currentStripeStart += stripeLength;
        }
        distanceAlongPath += segmentLen;
    }
}