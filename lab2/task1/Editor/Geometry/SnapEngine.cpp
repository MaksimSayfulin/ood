#include "SnapEngine.h"

#include <algorithm>
#include <cmath>
#include <limits>

editor::SnapEngine::Snap editor::SnapEngine::Find(const gfx::Rect& moving, const std::vector<gfx::Rect>& others, const int edges) {
    Snap snap;

    if (others.empty()) {
        return snap;
    }

    const double mxLeft = moving.x;
    const double mxCenter = moving.Center().x;
    const double mxRight = moving.Right();
    const double myTop = moving.y;
    const double myCenter = moving.Center().y;
    const double myBottom = moving.Bottom();

    double bestXDelta = 0.0;
    double bestXPos = 0.0;
    double bestXDist = std::numeric_limits<double>::max();

    double bestYDelta = 0.0;
    double bestYPos = 0.0;
    double bestYDist = std::numeric_limits<double>::max();

    for (const auto& other : others) {
        const double oxLeft = other.x;
        const double oxCenter = other.Center().x;
        const double oxRight = other.Right();
        const double oyTop = other.y;
        const double oyCenter = other.Center().y;
        const double oyBottom = other.Bottom();

        if ((edges & Left) != 0) {
            for (double ox : {oxLeft, oxCenter, oxRight}) {
                const double d = ox - mxLeft;
                const double ad = std::abs(d);
                if (ad < bestXDist && ad <= tolerance) {
                    bestXDist = ad;
                    bestXDelta = d;
                    bestXPos = ox;
                }
            }
        }
        if ((edges & CenterX) != 0) {
            for (double ox : {oxLeft, oxCenter, oxRight}) {
                const double d = ox - mxCenter;
                const double ad = std::abs(d);
                if (ad < bestXDist && ad <= tolerance) {
                    bestXDist = ad;
                    bestXDelta = d;
                    bestXPos = ox;
                }
            }
        }
        if ((edges & Right) != 0) {
            for (double ox : {oxLeft, oxCenter, oxRight}) {
                const double d = ox - mxRight;
                const double ad = std::abs(d);
                if (ad < bestXDist && ad <= tolerance) {
                    bestXDist = ad;
                    bestXDelta = d;
                    bestXPos = ox;
                }
            }
        }

        if ((edges & Top) != 0) {
            for (double oy : {oyTop, oyCenter, oyBottom}) {
                const double d = oy - myTop;
                const double ad = std::abs(d);
                if (ad < bestYDist && ad <= tolerance) {
                    bestYDist = ad;
                    bestYDelta = d;
                    bestYPos = oy;
                }
            }
        }
        if ((edges & CenterY) != 0) {
            for (double oy : {oyTop, oyCenter, oyBottom}) {
                const double d = oy - myCenter;
                const double ad = std::abs(d);
                if (ad < bestYDist && ad <= tolerance) {
                    bestYDist = ad;
                    bestYDelta = d;
                    bestYPos = oy;
                }
            }
        }
        if ((edges & Bottom) != 0) {
            for (double oy : {oyTop, oyCenter, oyBottom}) {
                const double d = oy - myBottom;
                const double ad = std::abs(d);
                if (ad < bestYDist && ad <= tolerance) {
                    bestYDist = ad;
                    bestYDelta = d;
                    bestYPos = oy;
                }
            }
        }
    }

    if (bestXDist <= tolerance) {
        snap.dx = bestXDelta;
        snap.guides.push_back(GuideLine{GuideLine::Axis::Vertical, bestXPos});
    }

    if (bestYDist <= tolerance) {
        snap.dy = bestYDelta;
        snap.guides.push_back(GuideLine{GuideLine::Axis::Horizontal, bestYPos});
    }

    return snap;
}
