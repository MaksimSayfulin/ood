#pragma once

#include <array>

#include "../../Graphics/Geometry/Rect.h"
#include "../Controller/EditorTypes.h"

namespace editor {
    class SelectionFrame {
    public:
        static constexpr double handleSize = 9.0;
        static constexpr double handleGrabSize = 18.0;
        static constexpr double frameGrabWidth = 6.0;
        static constexpr double overhang = handleGrabSize / 2;

        [[nodiscard]] static gfx::Rect GetFrameBounds(const gfx::Rect& shapeBounds);

        [[nodiscard]] static std::array<gfx::Point, 8> GetHandleCenters(const gfx::Rect& shapeBounds);

        [[nodiscard]] static gfx::Point GetHandleCenter(const gfx::Rect& shapeBounds, ResizeHandle handle);

        [[nodiscard]] static gfx::Rect GetHandleGrabBounds(const gfx::Point& center);

        [[nodiscard]] static ResizeHandle HitTestHandle(const gfx::Rect& shapeBounds, const gfx::Point& point);

        [[nodiscard]] static bool HitTestFrame(const gfx::Rect& shapeBounds, const gfx::Point& point);

        [[nodiscard]] static gfx::Rect ApplyHandle(const gfx::Rect& startBounds, ResizeHandle handle,
                                                   const gfx::Point& delta, double minSize,
                                                   const gfx::Size& canvasSize);
    };
}