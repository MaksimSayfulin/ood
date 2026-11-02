#include "EditorLayout.h"

#include <algorithm>

namespace {
    constexpr double groupGap = 24.0;
}

namespace {
    constexpr double colorSwatchSize = 20.0;
    constexpr double colorSwatchGap = 4.0;
}

const std::vector<editor::layout::ToolbarButton> &editor::layout::Toolbar() {
    static const std::vector<ToolbarButton> buttons = [] {
        const std::pair<UiCommand, std::string_view> items[] = {
            {UiCommand::Select, "Курсор"},
            {UiCommand::DrawRectangle, "Прямоуг."},
            {UiCommand::DrawEllipse, "Эллипс"},
            {UiCommand::DrawTriangle, "Треуг."},
        };

        const std::pair<UiCommand, gfx::Color> colors[] = {
            {UiCommand::ColorRed, gfx::Color{std::string{"#E6194B"}}},
            {UiCommand::ColorGreen, gfx::Color{std::string{"#3CB44B"}}},
            {UiCommand::ColorBlue, gfx::Color{std::string{"#4363D8"}}},
            {UiCommand::ColorBlack, gfx::Color{std::string{"#000000"}}},
            {UiCommand::ColorWhite, gfx::Color{std::string{"#FFFFFF"}}},
        };

        std::vector<ToolbarButton> result;
        double x = buttonPadding;

        for (const auto& [command, label] : items) {
            const gfx::Rect bounds{x, (toolbarHeight - buttonHeight) / 2, buttonWidth, buttonHeight};

            result.push_back({command, label, std::nullopt, bounds});
            x += buttonWidth + buttonGap;
        }

        x += groupGap;

        for (const auto& [command, color] : colors) {
            const gfx::Rect bounds{x, (toolbarHeight - colorSwatchSize) / 2, colorSwatchSize, colorSwatchSize};

            result.push_back({command, "", color, bounds});
            x += colorSwatchSize + colorSwatchGap;
        }

        x += groupGap;

        const std::pair<UiCommand, std::string_view> fileItems[] = {
            {UiCommand::DeleteShape, "Удалить"},
            {UiCommand::NewDocument, "Новый"},
            {UiCommand::OpenDocument, "Открыть"},
            {UiCommand::SaveDocument, "Сохранить"},
        };

        for (const auto& [command, label] : fileItems) {
            const gfx::Rect bounds{x, (toolbarHeight - buttonHeight) / 2, buttonWidth, buttonHeight};

            if (command == UiCommand::NewDocument) {
                x += groupGap;
            }

            result.push_back({command, label, std::nullopt, bounds});
            x += buttonWidth + buttonGap;
        }
        return result;
    }();

    return buttons;
}

std::optional<editor::UiCommand> editor::layout::HitTestToolbar(const gfx::Point& windowPoint) {
    for (const auto& button : Toolbar()) {
        if (button.bounds.Contains(windowPoint)) {
            return button.command;
        }
    }
    return std::nullopt;
}

gfx::Size editor::layout::InitialWindowSize(const gfx::Size& desktop) {
    const gfx::Size withMargin{desktop.width * desktopShare, desktop.height * desktopShare};

    return {std::clamp(withMargin.width, std::min(minWindowSize.width, desktop.width), desktop.width),
            std::clamp(withMargin.height, std::min(minWindowSize.height, desktop.height), desktop.height)};
}

gfx::Point editor::layout::WindowOrigin(const gfx::Size& window, const gfx::Size& desktop) {
    return {std::max(0.0, (desktop.width - window.width) / 2),
            std::max(0.0, (desktop.height - window.height) / 2)};
}

gfx::Rect editor::layout::CanvasBounds(const gfx::Size& size) {
    return {canvasMargin, toolbarHeight + canvasMargin,
            std::max(0.0, size.width - 2 * canvasMargin - shapeListWidth),
            std::max(0.0, size.height - toolbarHeight - statusHeight - 2 * canvasMargin)};
}

gfx::Rect editor::layout::StatusBarBounds(const gfx::Size& size) {
    return {0, size.height - statusHeight, size.width, statusHeight};
}

gfx::Rect editor::layout::ShapeListBounds(const gfx::Size& size) {
    const double width = std::min(shapeListWidth, std::max(0.0, size.width - 2 * canvasMargin));
    const double height = std::max(0.0, size.height - toolbarHeight - statusHeight - 2 * canvasMargin);

    return {std::max(0.0, size.width - canvasMargin - width), toolbarHeight + canvasMargin, width, height};
}

gfx::Rect editor::layout::ShapeListRowsBounds(const gfx::Size& size) {
    const auto panel = ShapeListBounds(size);
    const double scrollbar = std::min(shapeListScrollbarWidth, panel.width / 4);

    return {panel.x, panel.y + shapeListHeaderHeight,
            std::max(0.0, panel.width - scrollbar),
            std::max(0.0, panel.height - shapeListHeaderHeight)};
}

gfx::Rect editor::layout::ShapeListRowBounds(const gfx::Size& size, const std::size_t row, const double scroll) {
    const auto rows = ShapeListRowsBounds(size);

    return {rows.x, rows.y + static_cast<double>(row) * shapeListRowHeight - scroll, rows.width, shapeListRowHeight};
}

std::optional<std::size_t> editor::layout::HitTestShapeList(const gfx::Size& size, const gfx::Point& point,
                                                             const std::size_t rowCount, const double scroll) {
    const auto rows = ShapeListRowsBounds(size);
    if (!rows.Contains(point)) {
        return std::nullopt;
    }

    const double offset = point.y - rows.y + scroll;
    if (offset < 0) {
        return std::nullopt;
    }

    const auto row = static_cast<std::size_t>(offset / shapeListRowHeight);
    return row < rowCount ? std::optional<std::size_t>{row} : std::nullopt;
}