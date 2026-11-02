#include <string>
#include <vector>

#include "../Catch/Catch.hpp"
#include "../Editor/Controller/EditorController.h"
#include "../Editor/Geometry/SelectionFrame.h"
#include "../Graphics/Canvas/CanvasSVG/CanvasSvg.h"
#include "../Shape/model/ShapeFactory.h"

namespace {
    constexpr gfx::Size canvas{800, 600};

    struct Editor {
        std::ostringstream svg;
        gfx::CanvasSvg canvasOutput;
        shapes::Picture document;
        editor::EditorController controller;

        Editor() : canvasOutput(svg), document(canvasOutput), controller(document) {
            controller.OnCanvasResized(canvas);
        }

        void Add(const std::string& id, const std::string& type, const gfx::Rect& bounds) {
            document.AddShape({id, gfx::Color("#4363D8"), shapes::ShapeFactory::CreateGeometry(type, bounds)});
        }

        [[nodiscard]] gfx::Rect BoundsOf(const std::string& id) const {
            return document.GetShapeBounds(id);
        }
    };

    void SelectShape(Editor& editor, const gfx::Point& point, const bool toggle = false) {
        editor.controller.OnPointerPressed(point, toggle);
        editor.controller.OnPointerReleased(point);
    }
}

TEST_CASE("Множественное выделение: Ctrl+щелчок меняет состав выделения") {
    Editor editor;
    editor.Add("первая", "rectangle", {100, 100, 100, 100});
    editor.Add("вторая", "ellipse", {300, 300, 100, 100});

    SelectShape(editor, {150, 150});
    REQUIRE(editor.controller.GetSelectedIds() == std::vector<std::string>{"первая"});

    SECTION("Ctrl+щелчок по другой фигуре добавляет её в выделение") {
        SelectShape(editor, {350, 350}, true);

        REQUIRE(editor.controller.GetSelectedIds().size() == 2);
        REQUIRE(editor.controller.GetSelectedId() == "вторая");
    }

    SECTION("Ctrl+щелчок по выделенной фигуре убирает её из выделения") {
        SelectShape(editor, {150, 150}, true);

        REQUIRE(editor.controller.GetSelectedIds().empty());
        REQUIRE_FALSE(editor.controller.GetSelectedId().has_value());
    }

    SECTION("Обычный щелчок оставляет только одну фигуру") {
        SelectShape(editor, {350, 350}, true);
        SelectShape(editor, {150, 150});

        REQUIRE(editor.controller.GetSelectedIds() == std::vector<std::string>{"первая"});
    }

    SECTION("Щелчок по пустому месту снимает выделение") {
        SelectShape(editor, {350, 350}, true);
        SelectShape(editor, {600, 500});

        REQUIRE(editor.controller.GetSelectedIds().empty());
    }
}

TEST_CASE("Множественное выделение: перетаскивание перемещает всю группу") {
    Editor editor;
    editor.Add("первая", "rectangle", {100, 100, 100, 100});
    editor.Add("вторая", "ellipse", {300, 300, 100, 100});

    SelectShape(editor, {150, 150});
    SelectShape(editor, {350, 350}, true);
    REQUIRE(editor.controller.GetSelectedIds().size() == 2);

    editor.controller.OnPointerPressed({350, 350});
    editor.controller.OnPointerMoved({370, 330});
    editor.controller.OnPointerReleased({370, 330});

    const auto first = editor.BoundsOf("первая");
    const auto second = editor.BoundsOf("вторая");

    REQUIRE(first.x == 120);
    REQUIRE(first.y == 80);
    REQUIRE(second.x == 320);
    REQUIRE(second.y == 280);

    REQUIRE(second.x - first.x == 200);
    REQUIRE(second.y - first.y == 200);
}

TEST_CASE("Множественное выделение: группа не выходит за пределы холста") {
    Editor editor;
    editor.Add("первая", "rectangle", {0, 0, 100, 100});
    editor.Add("вторая", "ellipse", {700, 500, 100, 100});

    SelectShape(editor, {50, 50});
    SelectShape(editor, {750, 550}, true);

    editor.controller.OnPointerPressed({50, 50});
    editor.controller.OnPointerMoved({-500, -500});
    editor.controller.OnPointerReleased({-500, -500});

    REQUIRE(editor.BoundsOf("первая").x == 0);
    REQUIRE(editor.BoundsOf("первая").y == 0);
    REQUIRE(editor.BoundsOf("вторая").Right() == canvas.width);
    REQUIRE(editor.BoundsOf("вторая").Bottom() == canvas.height);
}

TEST_CASE("Множественное выделение: изменение размера одной фигуры масштабирует остальные") {
    Editor editor;
    editor.Add("первая", "rectangle", {100, 100, 100, 100});
    editor.Add("вторая", "ellipse", {300, 100, 100, 100});

    SelectShape(editor, {150, 150});
    SelectShape(editor, {350, 150}, true);

    const auto handle = editor::SelectionFrame::GetHandleCenter(
        editor.BoundsOf("первая"), editor::ResizeHandle::BottomRight);

    editor.controller.OnPointerPressed(handle);
    editor.controller.OnPointerMoved({handle.x + 100, handle.y + 100});
    editor.controller.OnPointerReleased({handle.x + 100, handle.y + 100});

    const auto first = editor.BoundsOf("первая");
    const auto second = editor.BoundsOf("вторая");

    REQUIRE(first.width == 200);
    REQUIRE(first.height == 200);

    REQUIRE(second.width == 200);
    REQUIRE(second.height == 200);
    REQUIRE(second.x == 500);
    REQUIRE(second.x - first.x == 400);
    REQUIRE(second.y == first.y);
}

TEST_CASE("Множественное выделение: Delete удаляет все выделенные фигуры") {
    Editor editor;
    editor.Add("первая", "rectangle", {100, 100, 100, 100});
    editor.Add("вторая", "ellipse", {300, 300, 100, 100});
    editor.Add("третья", "triangle", {500, 500, 100, 100});

    SelectShape(editor, {150, 150});
    SelectShape(editor, {350, 350}, true);

    editor.controller.OnCommand(editor::UiCommand::DeleteShape);

    REQUIRE(editor.document.GetShapesCount() == 1);
    REQUIRE(editor.document.GetShapes().front().GetId() == "третья");
    REQUIRE(editor.controller.GetSelectedIds().empty());
    REQUIRE_FALSE(editor.controller.GetSelectedId().has_value());
}

TEST_CASE("Множественное выделение: рамка выделения группы охватывает все фигуры") {
    Editor editor;
    editor.Add("первая", "rectangle", {100, 100, 100, 100});
    editor.Add("вторая", "ellipse", {300, 300, 100, 100});

    SelectShape(editor, {150, 150});
    SelectShape(editor, {350, 350}, true);

    const auto bounds = editor.controller.GetSelectionBounds();
    REQUIRE(bounds.has_value());
    REQUIRE(bounds->x == 100);
    REQUIRE(bounds->y == 100);
    REQUIRE(bounds->Right() == 400);
    REQUIRE(bounds->Bottom() == 400);
}
