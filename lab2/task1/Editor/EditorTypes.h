#pragma once

#include <string>
#include <string_view>

#include "../Graphics/Color/Color.h"
#include "../Graphics/Geometry/Rect.h"

namespace editor {
    enum class Tool { Select, Rectangle, Ellipse, Triangle };

    enum class UiCommand {
        Select,
        DrawRectangle,
        DrawEllipse,
        DrawTriangle,
        DeleteShape,
        NewDocument,
        OpenDocument,
        SaveDocument
    };

    enum class ResizeHandle {
        None,
        TopLeft,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left
    };

    enum class FilePromptMode {
        Open,
        Save
    };

    struct FilePromptView {
        std::string title;
        std::u32string path;
    };

    inline constexpr double minShapeSize = 12.0;

    inline constexpr gfx::Size defaultShapeSize{140.0, 90.0};

    [[nodiscard]] constexpr std::string_view DefaultColorFor(Tool tool) {
        switch (tool) {
        case Tool::Rectangle: return "#4363D8";
        case Tool::Ellipse: return "#E6194B";
        case Tool::Triangle: return "#3CB44B";
        case Tool::Select: break;
        }
        return "#FFFFFF";
    }

    [[nodiscard]] constexpr std::string_view ToShapeType(Tool tool) {
        switch (tool) {
        case Tool::Rectangle: return "rectangle";
        case Tool::Ellipse: return "ellipse";
        case Tool::Triangle: return "triangle";
        case Tool::Select: break;
        }
        return "";
    }
}