#pragma once

#include <vector>

#include "../../Graphics/Geometry/Rect.h"

namespace editor {
    struct GuideLine {
        enum class Axis {
            Vertical,
            Horizontal
        };

        Axis axis = Axis::Vertical;
        double position = 0;
    };

    class SnapEngine {
    public:
        enum SnapEdge {
            Left = 1 << 0,
            CenterX = 1 << 1,
            Right = 1 << 2,
            Top = 1 << 3,
            CenterY = 1 << 4,
            Bottom = 1 << 5
        };

        struct Snap {
            double dx = 0;
            double dy = 0;
            std::vector<GuideLine> guides;
        };

        static constexpr double tolerance = 7.0;

        static constexpr int allEdges = Left | CenterX | Right | Top | CenterY | Bottom;

        [[nodiscard]] static Snap Find(const gfx::Rect& moving, const std::vector<gfx::Rect>& others, int edges = allEdges);
    };
}
