#include <filesystem>
#include <fstream>
#include <sstream>
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

TEST_CASE("UndoRedo: на пустой истории Undo и Redo безопасны") {
    Editor editor;

    editor.controller.Undo();
    REQUIRE(editor.controller.CanUndo() == false);
    REQUIRE(editor.controller.CanRedo() == false);

    editor.controller.Redo();
    REQUIRE(editor.controller.CanRedo() == false);
}

TEST_CASE("UndoRedo: добавление фигуры отменяется и повторяется как одна операция") {
    Editor editor;

    editor.controller.OnCommand(editor::UiCommand::DrawRectangle);
    editor.controller.OnPointerPressed({100, 100});
    editor.controller.OnPointerMoved({300, 250});
    editor.controller.OnPointerReleased({300, 250});

    REQUIRE(editor.document.GetShapesCount() == 1);
    REQUIRE(editor.controller.CanUndo());

    editor.controller.Undo();

    REQUIRE(editor.document.GetShapesCount() == 0);
    REQUIRE_FALSE(editor.controller.CanUndo());
    REQUIRE(editor.controller.CanRedo());

    editor.controller.Redo();

    REQUIRE(editor.document.GetShapesCount() == 1);
    REQUIRE(editor.document.GetShapes().front().GetBounds() == gfx::Rect{100, 100, 200, 150});
}

TEST_CASE("UndoRedo: удаление фигуры отменяется и повторяется как одна операция") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {100, 100, 100, 100});
    SelectShape(editor, {150, 150});

    editor.controller.OnCommand(editor::UiCommand::DeleteShape);
    REQUIRE(editor.document.GetShapesCount() == 0);
    REQUIRE_FALSE(editor.controller.GetSelectedId().has_value());

    editor.controller.Undo();

    REQUIRE(editor.document.GetShapesCount() == 1);
    REQUIRE(editor.document.GetShapes().front().GetId() == "прямоугольник");
    REQUIRE(editor.BoundsOf("прямоугольник") == gfx::Rect{100, 100, 100, 100});

    editor.controller.Redo();

    REQUIRE(editor.document.GetShapesCount() == 0);
    REQUIRE_FALSE(editor.controller.GetSelectedId().has_value());
}

TEST_CASE("UndoRedo: Undo нескольких операций последовательно возвращает документ к состоянию до них") {
    Editor editor;

    editor.controller.OnCommand(editor::UiCommand::DrawRectangle);
    editor.controller.OnPointerPressed({100, 100});
    editor.controller.OnPointerMoved({300, 250});
    editor.controller.OnPointerReleased({300, 250});

    editor.controller.OnCommand(editor::UiCommand::DrawEllipse);
    editor.controller.OnPointerPressed({300, 300});
    editor.controller.OnPointerMoved({450, 400});
    editor.controller.OnPointerReleased({450, 400});

    REQUIRE(editor.document.GetShapesCount() == 2);

    editor.controller.Undo();
    REQUIRE(editor.document.GetShapesCount() == 1);

    editor.controller.Undo();
    REQUIRE(editor.document.GetShapesCount() == 0);
    REQUIRE_FALSE(editor.controller.CanUndo());

    editor.controller.Redo();
    REQUIRE(editor.document.GetShapesCount() == 1);

    editor.controller.Redo();
    REQUIRE(editor.document.GetShapesCount() == 2);
}

TEST_CASE("UndoRedo: после Undo нового действия Redo не восстанавливает старые команды") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {100, 100, 200, 100});

    editor.controller.OnPointerPressed({150, 150});
    editor.controller.OnPointerMoved({180, 150});
    editor.controller.OnPointerReleased({180, 150});
    REQUIRE(editor.BoundsOf("прямоугольник") == gfx::Rect{130, 100, 200, 100});

    editor.controller.Undo();
    REQUIRE(editor.BoundsOf("прямоугольник") == gfx::Rect{100, 100, 200, 100});
    REQUIRE(editor.controller.CanRedo());

    editor.controller.OnPointerPressed({150, 150});
    editor.controller.OnPointerMoved({200, 150});
    editor.controller.OnPointerReleased({200, 150});

    REQUIRE_FALSE(editor.controller.CanRedo());

    editor.controller.Redo();
    REQUIRE(editor.BoundsOf("прямоугольник") == gfx::Rect{150, 100, 200, 100});
}

TEST_CASE("UndoRedo: перемещение фигуры отменяется и повторяется как одна операция") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {100, 100, 200, 100});

    editor.controller.OnPointerPressed({150, 150});
    editor.controller.OnPointerMoved({180, 150});
    editor.controller.OnPointerReleased({180, 150});
    REQUIRE(editor.BoundsOf("прямоугольник") == gfx::Rect{130, 100, 200, 100});

    editor.controller.Undo();
    REQUIRE(editor.BoundsOf("прямоугольник") == gfx::Rect{100, 100, 200, 100});

    editor.controller.Redo();
    REQUIRE(editor.BoundsOf("прямоугольник") == gfx::Rect{130, 100, 200, 100});
}

TEST_CASE("UndoRedo: изменение размера фигуры отменяется и повторяется как одна операция") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {200, 200, 100, 100});

    editor.controller.OnPointerPressed({250, 250});
    editor.controller.OnPointerReleased({250, 250});

    const auto handle = editor::SelectionFrame::GetHandleCenter(
        editor.document.GetShapeBounds("прямоугольник"), editor::ResizeHandle::BottomRight);

    editor.controller.OnPointerPressed(handle);
    editor.controller.OnPointerMoved({handle.x + 60, handle.y + 40});
    editor.controller.OnPointerReleased({handle.x + 60, handle.y + 40});

    REQUIRE(editor.BoundsOf("прямоугольник") == gfx::Rect{200, 200, 160, 140});

    editor.controller.Undo();
    REQUIRE(editor.BoundsOf("прямоугольник") == gfx::Rect{200, 200, 100, 100});

    editor.controller.Redo();
    REQUIRE(editor.BoundsOf("прямоугольник") == gfx::Rect{200, 200, 160, 140});
}

TEST_CASE("UndoRedo: перетаскивание без фактического изменения не засоряет историю") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {100, 100, 200, 100});

    editor.controller.OnPointerPressed({150, 150});
    editor.controller.OnPointerReleased({150, 150});

    REQUIRE_FALSE(editor.controller.CanUndo());
}

TEST_CASE("UndoRedo: Undo нескольких удалений восстанавливает порядок фигур") {
    Editor editor;
    editor.Add("первая", "rectangle", {10, 10, 50, 50});
    editor.Add("вторая", "ellipse", {100, 100, 50, 50});
    editor.Add("третья", "rectangle", {200, 200, 50, 50});

    SelectShape(editor, {20, 20});
    SelectShape(editor, {220, 220}, true);

    REQUIRE(editor.controller.GetSelectedIds().size() == 2);

    editor.controller.OnCommand(editor::UiCommand::DeleteShape);
    REQUIRE(editor.document.GetShapesCount() == 1);

    editor.controller.Undo();
    REQUIRE(editor.document.GetShapesCount() == 3);

    REQUIRE(editor.document.GetShapes().front().GetId() == "первая");
    REQUIRE(editor.document.GetShapes().front().GetBounds() == gfx::Rect{10, 10, 50, 50});
    REQUIRE(editor.document.GetShapes().at(1).GetId() == "вторая");
    REQUIRE(editor.document.GetShapes().at(2).GetId() == "третья");

    editor.controller.Redo();
    REQUIRE(editor.document.GetShapesCount() == 1);
}

TEST_CASE("UndoRedo: удаление перемещённой фигуры отменяется и повторяется как одна операция") {
    Editor editor;

    editor.controller.OnCommand(editor::UiCommand::DrawRectangle);
    editor.controller.OnPointerPressed({100, 100});
    editor.controller.OnPointerMoved({300, 250});
    editor.controller.OnPointerReleased({300, 250});

    editor.controller.OnCommand(editor::UiCommand::Select);
    editor.controller.OnPointerPressed({150, 150});
    editor.controller.OnPointerMoved({180, 150});
    editor.controller.OnPointerReleased({180, 150});

    REQUIRE(editor.document.GetShapesCount() == 1);
    const auto id = *editor.controller.GetSelectedId();

    editor.controller.OnCommand(editor::UiCommand::DeleteShape);
    REQUIRE(editor.document.GetShapesCount() == 0);

    editor.controller.Undo();
    REQUIRE(editor.document.GetShapesCount() == 1);
    REQUIRE(editor.BoundsOf(id) == gfx::Rect{130, 100, 200, 150});

    editor.controller.Redo();
    REQUIRE(editor.document.GetShapesCount() == 0);
}

TEST_CASE("UndoRedo: сохранение и загрузка не затрагивают историю и корректно восстанавливают документ") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {10, 20, 100, 50});

    editor.controller.OnPointerPressed({50, 40});
    editor.controller.OnPointerReleased({50, 40});

    REQUIRE(editor.BoundsOf("прямоугольник") == gfx::Rect{10, 20, 100, 50});

    const auto path = std::filesystem::temp_directory_path() / "task4_undo_redo_test.shapes";
    std::filesystem::remove(path);

    REQUIRE(editor.controller.SaveDocument(path));

    editor.controller.NewDocument();
    REQUIRE(editor.controller.CanUndo() == false);
    REQUIRE(editor.controller.CanRedo() == false);
    REQUIRE(editor.document.GetShapesCount() == 0);

    REQUIRE(editor.controller.LoadDocument(path));
    REQUIRE(editor.document.GetShapesCount() == 1);
    REQUIRE(editor.document.GetShapes().front().GetId() == "прямоугольник");
    REQUIRE(editor.document.GetShapes().front().GetBounds() == gfx::Rect{10, 20, 100, 50});

    std::filesystem::remove(path);
}

TEST_CASE("UndoRedo: перестановка фигуры в списке слоёв отменяется и повторяется как одна операция") {
    Editor editor;
    editor.Add("нижняя", "rectangle", {100, 100, 100, 100});
    editor.Add("средняя", "ellipse", {200, 200, 100, 100});
    editor.Add("верхняя", "triangle", {300, 300, 100, 100});

    REQUIRE(editor.document.GetShapes().front().GetId() == "нижняя");
    REQUIRE(editor.document.GetShapes().back().GetId() == "верхняя");

    editor.controller.OnListRowPressed(2);
    editor.controller.OnListRowMoved(0);
    editor.controller.OnListRowReleased();

    REQUIRE(editor.document.GetShapes().front().GetId() == "средняя");
    REQUIRE(editor.document.GetShapes().back().GetId() == "нижняя");
    REQUIRE(editor.controller.CanUndo());

    editor.controller.Undo();

    REQUIRE(editor.document.GetShapes().front().GetId() == "нижняя");
    REQUIRE(editor.document.GetShapes().back().GetId() == "верхняя");

    editor.controller.Redo();

    REQUIRE(editor.document.GetShapes().front().GetId() == "средняя");
    REQUIRE(editor.document.GetShapes().back().GetId() == "нижняя");
}

TEST_CASE("UndoRedo: Redo после Undo до пустого холста не выбрасывает 'no such shape'") {
    Editor editor;

    editor.controller.OnCommand(editor::UiCommand::DrawRectangle);
    editor.controller.OnPointerPressed({100, 100});
    editor.controller.OnPointerMoved({300, 250});
    editor.controller.OnPointerReleased({300, 250});
    REQUIRE(editor.document.GetShapesCount() == 1);

    editor.controller.OnCommand(editor::UiCommand::DeleteShape);
    REQUIRE(editor.document.GetShapesCount() == 0);

    editor.controller.Undo();
    REQUIRE(editor.document.GetShapesCount() == 1);

    editor.controller.Undo();
    REQUIRE(editor.document.GetShapesCount() == 0);
    REQUIRE_FALSE(editor.controller.CanUndo());

    editor.controller.Redo();
    REQUIRE(editor.document.GetShapesCount() == 1);

    editor.controller.Redo();
    REQUIRE(editor.document.GetShapesCount() == 0);
    REQUIRE_FALSE(editor.controller.CanRedo());
}
