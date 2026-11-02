#include "SelectionFrame.h"

#include <algorithm>
#include <cmath>

namespace {
    bool IsLeftHandle(const editor::ResizeHandle handle) {
        using editor::ResizeHandle;
        return handle == ResizeHandle::TopLeft || handle == ResizeHandle::Left
            || handle == ResizeHandle::BottomLeft;
    }

    bool IsRightHandle(const editor::ResizeHandle handle) {
        using editor::ResizeHandle;
        return handle == ResizeHandle::TopRight || handle == ResizeHandle::Right
            || handle == ResizeHandle::BottomRight;
    }

    bool IsTopHandle(const editor::ResizeHandle handle) {
        using editor::ResizeHandle;
        return handle == ResizeHandle::TopLeft || handle == ResizeHandle::Top
            || handle == ResizeHandle::TopRight;
    }

    bool IsBottomHandle(const editor::ResizeHandle handle) {
        using editor::ResizeHandle;
        return handle == ResizeHandle::BottomLeft || handle == ResizeHandle::Bottom
            || handle == ResizeHandle::BottomRight;
    }
}

gfx::Rect editor::SelectionFrame::GetFrameBounds(const gfx::Rect& shapeBounds) {
    return shapeBounds;
}

std::array<gfx::Point, 8> editor::SelectionFrame::GetHandleCenters(const gfx::Rect& shapeBounds) {
    const auto frame = GetFrameBounds(shapeBounds);
    const double left = frame.x;
    const double right = frame.Right();
    const double top = frame.y;
    const double bottom = frame.Bottom();
    const double middleX = frame.Center().x;
    const double middleY = frame.Center().y;

    return {{{left, top},        {middleX, top},     {right, top},
             {right, middleY},  {right, bottom},   {middleX, bottom},
             {left, bottom},    {left, middleY}}};
}

gfx::Point editor::SelectionFrame::GetHandleCenter(const gfx::Rect& shapeBounds, const ResizeHandle handle) {
    if (handle == ResizeHandle::None) {
        return {};
    }

    const auto centers = GetHandleCenters(shapeBounds);
    const auto index = static_cast<std::size_t>(static_cast<int>(handle)
        - static_cast<int>(ResizeHandle::TopLeft));

    return centers[index];
}

gfx::Rect editor::SelectionFrame::GetHandleGrabBounds(const gfx::Point& center) {
    return {center.x - handleGrabSize / 2,
            center.y - handleGrabSize / 2,
            handleGrabSize,
            handleGrabSize};
}

editor::ResizeHandle editor::SelectionFrame::HitTestHandle(const gfx::Rect& shapeBounds,
                                                           const gfx::Point& point) {
    const auto centers = GetHandleCenters(shapeBounds);

    ResizeHandle hit = ResizeHandle::None;
    double bestDistance = 0;

    for (std::size_t i = 0; i < centers.size(); ++i) {
        if (!GetHandleGrabBounds(centers[i]).Contains(point)) {
            continue;
        }

        const double distance = std::hypot(point.x - centers[i].x, point.y - centers[i].y);
        if (hit == ResizeHandle::None || distance < bestDistance) {
            hit = static_cast<ResizeHandle>(static_cast<int>(ResizeHandle::TopLeft) + static_cast<int>(i));
            bestDistance = distance;
        }
    }

    return hit;
}

bool editor::SelectionFrame::HitTestFrame(const gfx::Rect& shapeBounds, const gfx::Point& point) {
    const gfx::Rect around{shapeBounds.x - frameGrabWidth,
                           shapeBounds.y - frameGrabWidth,
                           shapeBounds.width + 2 * frameGrabWidth,
                           shapeBounds.height + 2 * frameGrabWidth};

    return around.Contains(point) && !shapeBounds.Contains(point);
}

gfx::Rect editor::SelectionFrame::ApplyHandle(const gfx::Rect& startBounds, const ResizeHandle handle,
                                              const gfx::Point& delta, const double minSize,
                                              const gfx::Size& canvasSize) {
    double left = startBounds.x;
    double right = startBounds.Right();
    double top = startBounds.y;
    double bottom = startBounds.Bottom();

    if (IsLeftHandle(handle)) {
        left = std::max(left + delta.x, 0.0);
    } else if (IsRightHandle(handle)) {
        right = std::min(right + delta.x, canvasSize.width);
    }
    if (IsTopHandle(handle)) {
        top = std::max(top + delta.y, 0.0);
    } else if (IsBottomHandle(handle)) {
        bottom = std::min(bottom + delta.y, canvasSize.height);
    }

    if (IsLeftHandle(handle) && right - left < minSize) {
        left = right - minSize;
    } else if (IsRightHandle(handle) && right - left < minSize) {
        right = left + minSize;
    }
    if (IsTopHandle(handle) && bottom - top < minSize) {
        top = bottom - minSize;
    } else if (IsBottomHandle(handle) && bottom - top < minSize) {
        bottom = top + minSize;
    }

    return {left, top, right - left, bottom - top};
}