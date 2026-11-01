#include "SnapEngine.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace {
    struct Candidate {
        double delta = 0;
        double position = 0;
        double distance = std::numeric_limits<double>::max();
    };

    void Consider(const Candidate& candidate, Candidate& best) {
        if (candidate.distance > editor::SnapEngine::tolerance) {
            return;
        }

        if (candidate.distance < best.distance) {
            best = candidate;
        }
    }
}

editor::SnapEngine::Snap editor::SnapEngine::Find(const gfx::Rect& moving, const std::vector<gfx::Rect>& others, const int edges) {
    Snap snap;

    Candidate bestX;
    Candidate bestY;

    for (const auto& other : others) {
        const double otherX[3] = {other.x, other.Center().x, other.Right()};
        const double otherY[3] = {other.y, other.Center().y, other.Bottom()};

        const double movingX[3] = {moving.x, moving.Center().x, moving.Right()};
        const double movingY[3] = {moving.y, moving.Center().y, moving.Bottom()};

        const int xEdges[3] = {Left, CenterX, Right};
        const int yEdges[3] = {Top, CenterY, Bottom};

        for (int i = 0; i < 3; ++i) {
            if ((edges & xEdges[i]) != 0) {
                for (const double line : otherX) {
                    Consider({line - movingX[i], line, std::abs(line - movingX[i])}, bestX);
                }
            }

            if ((edges & yEdges[i]) != 0) {
                for (const double line : otherY) {
                    Consider({line - movingY[i], line, std::abs(line - movingY[i])}, bestY);
                }
            }
        }
    }

    if (bestX.distance <= tolerance) {
        snap.dx = bestX.delta;
        snap.guides.push_back(GuideLine{GuideLine::Axis::Vertical, bestX.position});
    }

    if (bestY.distance <= tolerance) {
        snap.dy = bestY.delta;
        snap.guides.push_back(GuideLine{GuideLine::Axis::Horizontal, bestY.position});
    }

    return snap;
}
