#pragma once

#include <cstddef>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "../Graphics/Geometry/Rect.h"
#include "EditorTypes.h"

namespace editor::layout {
    inline constexpr double toolbarHeight = 44.0;
    inline constexpr double statusHeight = 24.0;
    inline constexpr double canvasMargin = 6.0;

    inline constexpr double buttonWidth = 96.0;
    inline constexpr double buttonHeight = 28.0;
    inline constexpr double buttonPadding = 6.0;
    inline constexpr double buttonGap = 8.0;

    inline constexpr double shapeListWidth = 210.0;
    inline constexpr double shapeListHeaderHeight = 26.0;
    inline constexpr double shapeListRowHeight = 30.0;
    inline constexpr double shapeListIconSize = 20.0;
    inline constexpr double shapeListPadding = 8.0;
    inline constexpr double shapeListScrollbarWidth = 8.0;

    inline constexpr double desktopShare = 0.97;
    inline constexpr gfx::Size minWindowSize{900.0, 600.0};

    struct ToolbarButton {
        UiCommand command;
        std::string_view label;
        gfx::Rect bounds;
    };

    [[nodiscard]] const std::vector<ToolbarButton>& Toolbar();

    [[nodiscard]] std::optional<UiCommand> HitTestToolbar(const gfx::Point& windowPoint);

    [[nodiscard]] gfx::Size InitialWindowSize(const gfx::Size& desktop);

    [[nodiscard]] gfx::Point WindowOrigin(const gfx::Size& window, const gfx::Size& desktop);

    [[nodiscard]] gfx::Size InitialWindowSize(const gfx::Size& desktop);

    [[nodiscard]] gfx::Rect CanvasBounds(const gfx::Size& windowSize);

    [[nodiscard]] gfx::Rect StatusBarBounds(const gfx::Size& windowSize);

    [[nodiscard]] gfx::Rect ShapeListBounds(const gfx::Size& windowSize);
    [[nodiscard]] gfx::Rect ShapeListRowsBounds(const gfx::Size& windowSize);

    [[nodiscard]] gfx::Rect ShapeListRowBounds(const gfx::Size& windowSize, std::size_t row, double scroll);

    [[nodiscard]] std::optional<std::size_t> HitTestShapeList(const gfx::Size& windowSize,
                                                              const gfx::Point& windowPoint,
                                                              std::size_t rowCount, double scroll);
}