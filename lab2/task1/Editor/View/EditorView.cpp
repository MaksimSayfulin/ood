#include "EditorView.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <numbers>
#include <optional>
#include <string>

#include "../../Picture/view/SfmlCanvas.h"
#include "../../Picture/view/SfmlColor.h"
#include "../../Shape/model/ShapeFactory.h"
#include "EditorLayout.h"
#include "../Geometry/SelectionFrame.h"

namespace {
    constexpr sf::Color windowBackground{48, 50, 58};
    constexpr sf::Color toolbarBackground{62, 65, 75};
    constexpr sf::Color buttonBackground{86, 90, 102};
    constexpr sf::Color buttonActive{70, 130, 180};
    constexpr sf::Color textColor{226, 228, 234};
    constexpr sf::Color textMuted{150, 154, 166};

    constexpr sf::Color canvasBackground{252, 252, 250};
    constexpr sf::Color frameColor{70, 130, 180};
    constexpr sf::Color handleColor{255, 255, 255};
    constexpr sf::Color handleBorder{45, 45, 55};

    constexpr sf::Color listBackground{54, 57, 66};
    constexpr sf::Color listHeaderBackground{62, 65, 75};
    constexpr sf::Color listSelectionBackground{70, 96, 140};
    constexpr sf::Color listScrollTrack{44, 46, 54};
    constexpr sf::Color listScrollThumb{120, 124, 136};

    constexpr sf::Color guideColor{255, 92, 138, 220};

    constexpr double dashedLength = 6.0;

    gfx::Rect Offset(const gfx::Rect& rect, const gfx::Rect& canvasBounds) {
        return {rect.x + canvasBounds.x, rect.y + canvasBounds.y, rect.width, rect.height};
    }

    std::string_view ToolName(editor::Tool tool) {
        switch (tool) {
        case editor::Tool::Rectangle: return "Прямоугольник";
        case editor::Tool::Ellipse: return "Эллипс";
        case editor::Tool::Triangle: return "Треугольник";
        case editor::Tool::Select: break;
        }
        return "";
    }

    void DrawDashedRect(sf::RenderTarget& target, const gfx::Rect& bounds, const sf::Color& color) {
        const sf::Vector2f position{static_cast<float>(bounds.x), static_cast<float>(bounds.y)};
        const sf::Vector2f size{static_cast<float>(bounds.width), static_cast<float>(bounds.height)};

        sf::VertexArray segments(sf::PrimitiveType::Lines);
        const sf::Vector2f corners[4] = {
            position, position + sf::Vector2f{size.x, 0.f}, position + size, position + sf::Vector2f{0.f, size.y}
        };

        for (int side = 0; side < 4; ++side) {
            const auto from = corners[side];
            const auto to = corners[(side + 1) % 4];
            const double length = std::hypot(to.x - from.x, to.y - from.y);

            for (double offset = 0; offset < length; offset += dashedLength * 2) {
                const double dashEnd = std::min(offset + dashedLength, length);
                const auto mix = [](double from, double to, double t) { return from + (to - from) * t; };

                segments.append({{static_cast<float>(mix(from.x, to.x, offset / length)),
                                  static_cast<float>(mix(from.y, to.y, offset / length))}, color});
                segments.append({{static_cast<float>(mix(from.x, to.x, dashEnd / length)),
                                  static_cast<float>(mix(from.y, to.y, dashEnd / length))}, color});
            }
        }

        target.draw(segments);
    }
}

editor::EditorView::EditorView(const sf::Font* font) : m_font(font) {
}

void editor::EditorView::Render(sf::RenderTarget& target, const gfx::Size& windowSize,
                                const EditorController& controller, const shapes::Picture& document,
                                const std::string& statusMessage,
                                const std::optional<FilePromptView>& prompt) {
    const auto canvasBounds = layout::CanvasBounds(windowSize);

    target.clear(windowBackground);
    DrawToolbar(target, windowSize, controller.GetTool());
    DrawCanvas(target, canvasBounds, document);
    DrawSelectionFrame(target, canvasBounds, controller, document);
    DrawGuides(target, canvasBounds, controller);
    DrawShapeList(target, windowSize, controller, document);
    DrawDraft(target, canvasBounds, controller);
    DrawStatusBar(target, windowSize, controller, statusMessage);

    if (prompt) {
        DrawFilePrompt(target, windowSize, *prompt);
    }
}

void editor::EditorView::DrawToolbar(sf::RenderTarget& target, const gfx::Size& windowSize,
                                     const Tool activeTool) {
    FillRect(target, {0, 0, windowSize.width, layout::toolbarHeight}, toolbarBackground);

    const auto toolFor = [](UiCommand command) -> std::optional<Tool> {
        switch (command) {
        case UiCommand::Select: return Tool::Select;
        case UiCommand::DrawRectangle: return Tool::Rectangle;
        case UiCommand::DrawEllipse: return Tool::Ellipse;
        case UiCommand::DrawTriangle: return Tool::Triangle;
        default: return std::nullopt;
        }
    };

    for (const auto& button : layout::Toolbar()) {
        const bool active = toolFor(button.command) == activeTool;
        const auto bounds = button.bounds;

        if (button.color.has_value()) {
            const auto sfColor = gfx::ToSfmlColor(*button.color);
            FillRect(target, bounds, sfColor, 1.5f, active ? frameColor : handleBorder);
            continue;
        }

        FillRect(target, bounds, active ? buttonActive : buttonBackground, active ? 2.f : 1.f, frameColor);

        if (m_font != nullptr) {
            DrawLabel(target, button.label, 14, {bounds.x + 10, bounds.y + 7}, textColor);
        }
    }
}

void editor::EditorView::DrawCanvas(sf::RenderTarget& target, const gfx::Rect& canvasBounds,
                                    const shapes::Picture& document) {
    FillRect(target, canvasBounds, canvasBackground, 1.f, sf::Color{180, 180, 185});

    gfx::SfmlCanvas canvas(target, m_font);
    canvas.SetOffset({static_cast<float>(canvasBounds.x), static_cast<float>(canvasBounds.y)});
    document.DrawPicture(canvas);
}

void editor::EditorView::DrawSelectionFrame(sf::RenderTarget& target, const gfx::Rect& canvasBounds,
                                            const EditorController& controller,
                                            const shapes::Picture& document) {
    for (const auto& id : controller.GetSelectedIds()) {
        const auto bounds = document.GetShapeBounds(id);

        DrawDashedRect(target, Offset(SelectionFrame::GetFrameBounds(bounds), canvasBounds), frameColor);

        for (const auto& center : SelectionFrame::GetHandleCenters(bounds)) {
            FillRect(target, Offset({center.x - SelectionFrame::handleSize / 2,
                                     center.y - SelectionFrame::handleSize / 2,
                                     SelectionFrame::handleSize, SelectionFrame::handleSize}, canvasBounds),
                     handleColor, 1.5f, handleBorder);
        }
    }
}

void editor::EditorView::DrawGuides(sf::RenderTarget& target, const gfx::Rect& canvasBounds,
                                    const EditorController& controller) {
    const auto& guides = controller.GetActiveGuides();
    if (guides.empty()) {
        return;
    }

    sf::VertexArray lines(sf::PrimitiveType::Lines);

    for (const auto& guide : guides) {
        const auto position = static_cast<float>(guide.position);
        const auto from = static_cast<float>(guide.axis == GuideLine::Axis::Vertical
                                                   ? canvasBounds.y
                                                   : canvasBounds.x);
        const auto to = static_cast<float>(guide.axis == GuideLine::Axis::Vertical
                                                 ? canvasBounds.Bottom()
                                                 : canvasBounds.Right());

        const sf::Vector2f start = guide.axis == GuideLine::Axis::Vertical
                                       ? sf::Vector2f{position + static_cast<float>(canvasBounds.x), from}
                                       : sf::Vector2f{from, position + static_cast<float>(canvasBounds.y)};
        const sf::Vector2f end = guide.axis == GuideLine::Axis::Vertical
                                     ? sf::Vector2f{position + static_cast<float>(canvasBounds.x), to}
                                     : sf::Vector2f{to, position + static_cast<float>(canvasBounds.y)};

        lines.append({start, guideColor});
        lines.append({end, guideColor});
    }

    target.draw(lines);
}

void editor::EditorView::DrawShapeList(sf::RenderTarget& target, const gfx::Size& windowSize,
                                       const EditorController& controller,
                                       const shapes::Picture& document) {
    const auto panel = layout::ShapeListBounds(windowSize);
    const auto rows = layout::ShapeListRowsBounds(windowSize);
    const double scroll = controller.GetListScroll();

    FillRect(target, panel, listBackground);
    FillRect(target, {panel.x, panel.y, panel.width, layout::shapeListHeaderHeight}, listHeaderBackground);

    if (m_font != nullptr) {
        DrawLabel(target, "Слои", 14, {panel.x + layout::shapeListPadding, panel.y + 5}, textMuted);
    }

    const auto& shapes = document.GetShapes();
    const auto count = shapes.size();
    const auto& selected = controller.GetSelectedIds();

    for (std::size_t row = 0; row < count; ++row) {
        const auto bounds = layout::ShapeListRowBounds(windowSize, row, scroll);
        if (bounds.Bottom() < rows.y || bounds.y > rows.Bottom()) {
            continue;
        }

        const auto& shape = shapes[count - 1 - row];
        const bool isSelected = std::find(selected.begin(), selected.end(), shape.GetId()) != selected.end();

        if (isSelected) {
            FillRect(target, bounds, listSelectionBackground);
        }

        const auto icon = gfx::Rect{bounds.x + layout::shapeListPadding,
                                    bounds.y + (layout::shapeListRowHeight - layout::shapeListIconSize) / 2,
                                    layout::shapeListIconSize,
                                    layout::shapeListIconSize};
        DrawShapeIcon(target, icon, shape.GetGeometry()->GetShapeName(), shape.GetColor());

        if (m_font != nullptr) {
            DrawLabel(target, shape.GetId(), 13,
                      {icon.x + layout::shapeListIconSize + 6, bounds.y + 8}, textColor);
        }
    }

    const double content = static_cast<double>(count) * layout::shapeListRowHeight;
    if (content > rows.height && rows.height > 0) {
        const double thumbHeight = std::max(20.0, rows.height * rows.height / content);
        const double maxScroll = content - rows.height;
        const double position = maxScroll > 0 ? std::clamp(scroll / maxScroll, 0.0, 1.0) : 0.0;

        FillRect(target, {rows.Right(), rows.y, layout::shapeListScrollbarWidth, rows.height}, listScrollTrack);
        FillRect(target, {rows.Right(), rows.y + position * (rows.height - thumbHeight),
                          layout::shapeListScrollbarWidth, thumbHeight},
                 listScrollThumb);
    }
}

void editor::EditorView::DrawShapeIcon(sf::RenderTarget& target, const gfx::Rect& bounds,
                                       const std::string_view type, const gfx::Color& color) {
    const auto fill = gfx::ToSfmlColor(color);
    const auto position = sf::Vector2f{static_cast<float>(bounds.x), static_cast<float>(bounds.y)};
    const auto size = sf::Vector2f{static_cast<float>(bounds.width), static_cast<float>(bounds.height)};

    if (type == "ellipse") {
        const float radius = std::min(size.x, size.y) / 2;
        sf::CircleShape circle(radius);
        circle.setOrigin({radius, radius});
        circle.setPosition(position + sf::Vector2f{radius, radius});
        circle.setFillColor(fill);
        target.draw(circle);
        return;
    }

    if (type == "triangle") {
        sf::ConvexShape triangle;
        triangle.setPointCount(3);
        triangle.setPoint(0, {0.f, size.y});
        triangle.setPoint(1, {size.x / 2, 0.f});
        triangle.setPoint(2, {size.x, size.y});
        triangle.setPosition(position);
        triangle.setFillColor(fill);
        target.draw(triangle);
        return;
    }

    if (type == "line") {
        sf::RectangleShape line({std::hypot(size.x, size.y), 2.f});
        line.setOrigin({0.f, 1.f});
        line.setPosition(position);
        line.setRotation(sf::radians(std::atan2(size.y, size.x) * 180.0 / 3.14159265358979323846));
        line.setFillColor(fill);
        target.draw(line);
        return;
    }

    sf::RectangleShape rectangle(size);
    rectangle.setPosition(position);
    rectangle.setFillColor(fill);
    target.draw(rectangle);
}

void editor::EditorView::DrawDraft(sf::RenderTarget& target, const gfx::Rect& canvasBounds,
                                   const EditorController& controller) const {
    const auto draft = controller.GetDraftBounds();
    if (!draft) {
        return;
    }

    const auto tool = controller.GetTool();
    auto geometry = shapes::ShapeFactory::CreateGeometry(ToShapeType(tool), *draft);

    gfx::SfmlCanvas canvas(target, m_font);
    canvas.SetOffset({static_cast<float>(canvasBounds.x), static_cast<float>(canvasBounds.y)});
    canvas.SetOverrideColor(sf::Color{70, 130, 180, 110});
    geometry->Draw(canvas);
}

void editor::EditorView::DrawStatusBar(sf::RenderTarget& target, const gfx::Size& windowSize,
                                       const EditorController& controller,
                                       const std::string& statusMessage) const {
    const auto bounds = layout::StatusBarBounds(windowSize);
    FillRect(target, bounds, toolbarBackground);

    const auto& error = controller.GetLastError();
    const bool hasError = !error.empty();

    std::string text = statusMessage;
    if (hasError) {
        text = std::format("Ошибка: {}", error);
    } else if (controller.GetTool() != Tool::Select) {
        text = std::format("{}: нажмите и потяните мышью, Esc - отмена", ToolName(controller.GetTool()));
    }

    if (m_font != nullptr) {
        DrawLabel(target, text, 13, {bounds.x + 8, bounds.y + 4},
                  hasError ? sf::Color{255, 140, 130} : textMuted);
    }

    const auto& selected = controller.GetSelectedIds();
    if (selected.size() == 1) {
        DrawLabel(target, std::format("выбрана: {}", selected.front()), 13,
                  {bounds.Right() - 190, bounds.y + 4}, textMuted);
    } else if (selected.size() > 1) {
        DrawLabel(target, std::format("выбрано фигур: {}", selected.size()), 13,
                  {bounds.Right() - 190, bounds.y + 4}, textMuted);
    }
}

void editor::EditorView::DrawFilePrompt(sf::RenderTarget& target, const gfx::Size& windowSize,
                                        const FilePromptView& prompt) {
    const gfx::Rect box{windowSize.width / 2 - 320, windowSize.height / 2 - 90, 640, 180};
    FillRect(target, {0, 0, windowSize.width, windowSize.height}, sf::Color{0, 0, 0, 140});
    FillRect(target, box, sf::Color{40, 42, 50}, 2.f, frameColor);

    if (m_font == nullptr) {
        return;
    }

    DrawLabel(target, prompt.title, 16, {box.x + 20, box.y + 18}, textColor);
    if (prompt.path.empty()) {
        DrawLabel(target, "<введите путь>", 15, {box.x + 20, box.y + 60}, textMuted);
    } else {
        DrawLabel(target, prompt.path, 15, {box.x + 20, box.y + 60}, textColor);
    }
    DrawLabel(target, "Enter - подтвердить, Esc - отмена, Backspace - удалить символ", 12,
              {box.x + 20, box.y + 120}, textMuted);
}

void editor::EditorView::DrawLabel(sf::RenderTarget& target, const std::string_view text,
                                   const unsigned size, const gfx::Point& position,
                                   const sf::Color color) const {
    if (m_font == nullptr) {
        return;
    }

    sf::Text label(*m_font, sf::String::fromUtf8(text.begin(), text.end()), size);
    label.setFillColor(color);
    label.setPosition({static_cast<float>(position.x), static_cast<float>(position.y)});

    target.draw(label);
}

void editor::EditorView::DrawLabel(sf::RenderTarget& target, const std::u32string& text,
                                   const unsigned size, const gfx::Point& position,
                                   const sf::Color color) const {
    sf::Text label(*m_font, sf::String{text}, size);
    label.setFillColor(color);
    label.setPosition({static_cast<float>(position.x), static_cast<float>(position.y)});

    target.draw(label);
}

void editor::EditorView::FillRect(sf::RenderTarget& target, const gfx::Rect& bounds, const sf::Color color,
                                  const float outline, const sf::Color outlineColor) const {
    sf::RectangleShape rectangle({static_cast<float>(bounds.width), static_cast<float>(bounds.height)});
    rectangle.setPosition({static_cast<float>(bounds.x), static_cast<float>(bounds.y)});
    rectangle.setFillColor(color);

    if (outline > 0.f) {
        rectangle.setOutlineThickness(outline);
        rectangle.setOutlineColor(outlineColor);
    }

    target.draw(rectangle);
}