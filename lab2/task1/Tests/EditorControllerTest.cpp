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
}

TEST_CASE("Выделение: щелчок по фигуре выбирает её, щелчок по пустому месту снимает") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {100, 100, 200, 100});

    SECTION("Щелчок внутри фигуры выделяет её") {
        editor.controller.OnPointerPressed({150, 150});
        REQUIRE(editor.controller.GetSelectedId() == "прямоугольник");
    }

    SECTION("Щелчок по свободной области снимает выделение") {
        editor.controller.OnPointerPressed({150, 150});
        editor.controller.OnPointerReleased({150, 150});

        editor.controller.OnPointerPressed({700, 500});
        REQUIRE_FALSE(editor.controller.GetSelectedId().has_value());
    }
}

TEST_CASE("Выделение: учитывается настоящая форма фигуры, а не ограничивающий прямоугольник") {
    Editor editor;
    editor.Add("эллипс", "ellipse", {100, 100, 200, 200});

    SECTION("Щелчок в углу границ эллипса не выбирает фигуру") {
        editor.controller.OnPointerPressed({105, 105});
        REQUIRE_FALSE(editor.controller.GetSelectedId().has_value());
    }

    SECTION("Щелчок в центре эллипса выбирает фигуру") {
        editor.controller.OnPointerPressed({200, 200});
        REQUIRE(editor.controller.GetSelectedId() == "эллипс");
    }

    SECTION("У треугольника тоже выбирается только настоящая форма") {
        Editor editor;
        editor.Add("треугольник", "triangle", {0, 0, 200, 200});

        editor.controller.OnPointerPressed({100, 180});
        REQUIRE(editor.controller.GetSelectedId() == "треугольник");

        editor.controller.OnPointerReleased({100, 180});
        editor.controller.OnPointerPressed({10, 10});
        REQUIRE_FALSE(editor.controller.GetSelectedId().has_value());
    }
}

TEST_CASE("Выделение: из перекрывающихся фигур выбирается верхняя") {
    Editor editor;
    editor.Add("нижняя", "rectangle", {100, 100, 200, 200});
    editor.Add("верхняя", "rectangle", {150, 150, 200, 200});

    editor.controller.OnPointerPressed({200, 200});

    REQUIRE(editor.controller.GetSelectedId() == "верхняя");
}

TEST_CASE("Перемещение: фигура сохраняет положение относительно курсора") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {100, 100, 200, 100});

    SECTION("Захват не за центр - фигура не прыгает под курсор") {
        editor.controller.OnPointerPressed({150, 130});
        editor.controller.OnPointerMoved({180, 150});
        editor.controller.OnPointerReleased({180, 150});

        const auto bounds = editor.BoundsOf("прямоугольник");
        REQUIRE(bounds.x == 130);
        REQUIRE(bounds.y == 120);
    }

    SECTION("Перетаскивание за рамку выделения работает так же") {
        editor.controller.OnPointerPressed({150, 150});
        editor.controller.OnPointerReleased({150, 150});

        const auto framePoint = gfx::Point{150, 100 - editor::SelectionFrame::frameGrabWidth + 1};
        editor.controller.OnPointerPressed(framePoint);
        editor.controller.OnPointerMoved({200, 90});
        editor.controller.OnPointerReleased({200, 90});

        const auto bounds = editor.BoundsOf("прямоугольник");
        REQUIRE(bounds.x == 150);
        REQUIRE(bounds.y == 95);
    }
}

TEST_CASE("Перемещение: фигура не выходит за пределы холста") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {100, 100, 200, 100});

    editor.controller.OnPointerPressed({150, 150});
    editor.controller.OnPointerMoved({5000, 5000});
    editor.controller.OnPointerReleased({5000, 5000});

    const auto bounds = editor.BoundsOf("прямоугольник");
    REQUIRE(bounds.Right() == canvas.width);
    REQUIRE(bounds.Bottom() == canvas.height);

    editor.controller.OnPointerPressed({canvas.width - 50, canvas.height - 50});
    editor.controller.OnPointerMoved({-5000, -5000});
    editor.controller.OnPointerReleased({-5000, -5000});

    REQUIRE(editor.BoundsOf("прямоугольник").x == 0);
    REQUIRE(editor.BoundsOf("прямоугольник").y == 0);
}

TEST_CASE("Размеры: маркеры рамки меняют границы фигуры") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {200, 200, 100, 100});

    const gfx::Rect bounds{200, 200, 100, 100};
    const gfx::Point bottomRight = editor::SelectionFrame::GetHandleCenter(
        bounds, editor::ResizeHandle::BottomRight);
    const gfx::Point topLeft = editor::SelectionFrame::GetHandleCenter(
        bounds, editor::ResizeHandle::TopLeft);

    SECTION("Протяжка за правый нижний маркер") {
        editor.controller.OnPointerPressed({200, 200});
        editor.controller.OnPointerReleased({200, 200});

        editor.controller.OnPointerPressed(bottomRight);
        editor.controller.OnPointerMoved({360, 340});
        editor.controller.OnPointerReleased({360, 340});

        const auto bounds = editor.BoundsOf("прямоугольник");
        REQUIRE(bounds.x == 200);
        REQUIRE(bounds.y == 200);
        REQUIRE(bounds.width == 160);
        REQUIRE(bounds.height == 140);
    }

    SECTION("Протяжка за левый верхний маркер двигает начало границ") {
        editor.controller.OnPointerPressed({200, 200});
        editor.controller.OnPointerReleased({200, 200});

        editor.controller.OnPointerPressed(topLeft);
        editor.controller.OnPointerMoved({170, 150});
        editor.controller.OnPointerReleased({170, 150});

        const auto bounds = editor.BoundsOf("прямоугольник");
        REQUIRE(bounds.x == 170);
        REQUIRE(bounds.y == 150);
        REQUIRE(bounds.width == 130);
        REQUIRE(bounds.height == 150);
    }

    SECTION("Фигура не становится меньше минимального размера") {
        editor.controller.OnPointerPressed({200, 200});
        editor.controller.OnPointerReleased({200, 200});

        editor.controller.OnPointerPressed(bottomRight);
        editor.controller.OnPointerMoved({100, 100});
        editor.controller.OnPointerReleased({100, 100});

        const auto bounds = editor.BoundsOf("прямоугольник");
        REQUIRE(bounds.width == editor::minShapeSize);
        REQUIRE(bounds.height == editor::minShapeSize);
    }

    SECTION("Изменение размера не выводит фигуру за пределы холста") {
        editor.Add("вторая", "rectangle", {600, 400, 150, 150});

        editor.controller.OnPointerPressed({200, 200});
        editor.controller.OnPointerReleased({200, 200});

        editor.controller.OnPointerPressed(bottomRight);
        editor.controller.OnPointerMoved({5000, 5000});
        editor.controller.OnPointerReleased({5000, 5000});

        const auto bounds = editor.BoundsOf("прямоугольник");
        REQUIRE(bounds.x == 200);
        REQUIRE(bounds.y == 200);
        REQUIRE(bounds.Right() == canvas.width);
        REQUIRE(bounds.Bottom() == canvas.height);
    }

    SECTION("Маркер не прыгает под курсор, если его взяли не за центр") {
        editor.controller.OnPointerPressed({200, 200});
        editor.controller.OnPointerReleased({200, 200});

        editor.controller.OnPointerPressed({199, 199});
        editor.controller.OnPointerMoved({150, 150});
        editor.controller.OnPointerReleased({150, 150});

        const auto bounds = editor.BoundsOf("прямоугольник");
        REQUIRE(bounds.x == 151);
        REQUIRE(bounds.y == 151);

        REQUIRE(editor::SelectionFrame::GetHandleCenter(bounds, editor::ResizeHandle::TopLeft)
                == (gfx::Point{151, 151}));
    }

    SECTION("Упор левого верхнего маркера в холст не двигает правый нижний угол") {
        editor.controller.OnPointerPressed({200, 200});
        editor.controller.OnPointerReleased({200, 200});

        editor.controller.OnPointerPressed(topLeft);
        editor.controller.OnPointerMoved({-100, -100});
        editor.controller.OnPointerReleased({-100, -100});

        const auto bounds = editor.BoundsOf("прямоугольник");
        REQUIRE(bounds.x == 0);
        REQUIRE(bounds.y == 0);
        REQUIRE(bounds.width == 300);
        REQUIRE(bounds.height == 300);
    }
}

TEST_CASE("Размеры: ресайз работает по всем восьми маркерам, угол тянет обе границы") {
    struct Case {
        const char* name;
        editor::ResizeHandle handle;
        gfx::Point drag;
        gfx::Rect expected;
    };

    const gfx::Rect start{200, 200, 100, 100};
    const std::vector<Case> cases{
        {"верхний левый", editor::ResizeHandle::TopLeft, {-20, -10}, {180, 190, 120, 110}},
        {"верхний правый", editor::ResizeHandle::TopRight, {20, -10}, {200, 190, 120, 110}},
        {"нижний левый", editor::ResizeHandle::BottomLeft, {-20, 10}, {180, 200, 120, 110}},
        {"нижний правый", editor::ResizeHandle::BottomRight, {20, 10}, {200, 200, 120, 110}},
        {"верхний", editor::ResizeHandle::Top, {0, -10}, {200, 190, 100, 110}},
        {"нижний", editor::ResizeHandle::Bottom, {0, 10}, {200, 200, 100, 110}},
        {"левый", editor::ResizeHandle::Left, {-20, 0}, {180, 200, 120, 100}},
        {"правый", editor::ResizeHandle::Right, {20, 0}, {200, 200, 120, 100}},
    };

    for (const auto& testCase : cases) {
        CAPTURE(testCase.name);

        Editor editor;
        editor.Add("прямоугольник", "rectangle", start);

        editor.controller.OnPointerPressed({250, 250});
        editor.controller.OnPointerReleased({250, 250});

        const auto center = editor::SelectionFrame::GetHandleCenter(start, testCase.handle);
        REQUIRE(editor::SelectionFrame::HitTestHandle(start, center) == testCase.handle);

        editor.controller.OnPointerPressed(center);
        editor.controller.OnPointerMoved({center.x + testCase.drag.x, center.y + testCase.drag.y});
        editor.controller.OnPointerReleased({center.x + testCase.drag.x, center.y + testCase.drag.y});

        REQUIRE(editor.BoundsOf("прямоугольник") == testCase.expected);
    }
}

TEST_CASE("Размеры: рамка остаётся доступной, когда фигура перекрыта другой") {
    Editor editor;
    editor.Add("выделенная", "rectangle", {200, 200, 200, 200});
    editor.controller.OnPointerPressed({300, 300});
    editor.controller.OnPointerReleased({300, 300});

    editor.Add("перекрывающая", "rectangle", {100, 100, 400, 400});

    const auto framePoint = gfx::Point{250, 200 - editor::SelectionFrame::frameGrabWidth + 1};
    editor.controller.OnPointerPressed(framePoint);
    editor.controller.OnPointerMoved({250, 120});
    editor.controller.OnPointerReleased({250, 120});

    REQUIRE(editor.controller.GetSelectedId() == "выделенная");
    REQUIRE(editor.BoundsOf("выделенная").y == 125);
    REQUIRE(editor.BoundsOf("выделенная").height == 200);
}

TEST_CASE("Удаление: команда Delete удаляет выделенную фигуру и снимает выделение") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {100, 100, 100, 100});
    editor.controller.OnPointerPressed({150, 150});
    editor.controller.OnPointerReleased({150, 150});

    editor.controller.OnCommand(editor::UiCommand::DeleteShape);

    REQUIRE(editor.document.GetShapesCount() == 0);
    REQUIRE_FALSE(editor.controller.GetSelectedId().has_value());
    REQUIRE_FALSE(editor.controller.GetSelectionBounds().has_value());
}

TEST_CASE("Инструменты: кнопка панели выбирает инструмент, протяжка создаёт фигуру") {
    Editor editor;

    SECTION("Кнопка панели только выбирает инструмент, не создаёт фигуру") {
        editor.controller.OnCommand(editor::UiCommand::DrawEllipse);

        REQUIRE(editor.document.GetShapesCount() == 0);
        REQUIRE(editor.controller.GetTool() == editor::Tool::Ellipse);
        REQUIRE_FALSE(editor.controller.GetSelectedId().has_value());
    }

    SECTION("Протяжка задаёт границы, новая фигура попадает на верхний слой") {
        editor.Add("первая", "rectangle", {0, 0, 50, 50});
        editor.controller.OnCommand(editor::UiCommand::DrawRectangle);

        editor.controller.OnPointerPressed({100, 100});
        editor.controller.OnPointerMoved({300, 250});

        REQUIRE(editor.controller.GetDraftBounds() == (gfx::Rect{100, 100, 200, 150}));

        editor.controller.OnPointerReleased({300, 250});

        REQUIRE(editor.document.GetShapesCount() == 2);
        REQUIRE(editor.document.GetShapes().back().GetBounds() == (gfx::Rect{100, 100, 200, 150}));
        REQUIRE(editor.controller.GetSelectedId() == editor.document.GetShapes().back().GetId());
    }

    SECTION("Протяжка влево и вверх даёт те же положительные размеры") {
        editor.controller.OnCommand(editor::UiCommand::DrawEllipse);
        editor.controller.OnPointerPressed({300, 300});
        editor.controller.OnPointerMoved({100, 100});
        editor.controller.OnPointerReleased({100, 100});

        REQUIRE(editor.document.GetShapes().back().GetBounds() == (gfx::Rect{100, 100, 200, 200}));
    }

    SECTION("Одиночный щелчок с нулевым размером не создаёт фигуру") {
        editor.controller.OnCommand(editor::UiCommand::DrawTriangle);
        editor.controller.OnPointerPressed({400, 300});
        editor.controller.OnPointerReleased({400, 300});

        REQUIRE(editor.document.GetShapesCount() == 0);
        REQUIRE_FALSE(editor.controller.CanUndo());
    }

    SECTION("Протяжка с нулевой шириной не создаёт фигуру") {
        editor.controller.OnCommand(editor::UiCommand::DrawTriangle);
        editor.controller.OnPointerPressed({400, 300});
        editor.controller.OnPointerMoved({400, 450});
        editor.controller.OnPointerReleased({400, 450});

        REQUIRE(editor.document.GetShapesCount() == 0);
        REQUIRE_FALSE(editor.controller.CanUndo());
    }

    SECTION("Инструмент остаётся активным: можно нарисовать следующую фигуру") {
        editor.controller.OnCommand(editor::UiCommand::DrawRectangle);
        editor.controller.OnPointerPressed({100, 100});
        editor.controller.OnPointerReleased({100, 100});

        REQUIRE(editor.controller.GetTool() == editor::Tool::Rectangle);
    }

    SECTION("Создание у края холста не выходит за его пределы") {
        editor.controller.OnCommand(editor::UiCommand::DrawRectangle);
        editor.controller.OnPointerPressed({canvas.width - 10, canvas.height - 10});
        editor.controller.OnPointerMoved({canvas.width + 100, canvas.height + 100});

        const auto draft = editor.controller.GetDraftBounds();
        REQUIRE(draft.has_value());
        REQUIRE(draft->Right() <= canvas.width);
        REQUIRE(draft->Bottom() <= canvas.height);

        editor.controller.OnPointerReleased({canvas.width + 100, canvas.height + 100});

        const auto bounds = editor.document.GetShapes().back().GetBounds();
        REQUIRE(bounds.Right() <= canvas.width);
        REQUIRE(bounds.Bottom() <= canvas.height);
    }

    SECTION("Esc отменяет незавершённое создание") {
        editor.controller.OnCommand(editor::UiCommand::DrawRectangle);
        editor.controller.OnPointerPressed({100, 100});
        editor.controller.OnPointerMoved({300, 250});

        REQUIRE(editor.controller.GetDraftBounds().has_value());

        editor.controller.OnCancel();

        REQUIRE_FALSE(editor.controller.GetDraftBounds().has_value());
        REQUIRE(editor.document.GetShapesCount() == 0);
    }

    SECTION("Esc отменяет незавершённое создание и снимает инструмент") {
        editor.controller.OnCommand(editor::UiCommand::DrawRectangle);
        const auto shapesBefore = editor.document.GetShapesCount();

        editor.controller.OnPointerPressed({100, 100});
        editor.controller.OnPointerMoved({200, 200});
        editor.controller.OnCancel();
        editor.controller.OnPointerReleased({200, 200});

        REQUIRE(editor.document.GetShapesCount() == shapesBefore);
        REQUIRE(editor.controller.GetTool() == editor::Tool::Select);
    }

    SECTION("Фигура около границы холста обрезается по холсту") {
        editor.controller.OnCommand(editor::UiCommand::DrawRectangle);
        editor.controller.OnPointerPressed({700, 500});
        editor.controller.OnPointerMoved({900, 700});
        editor.controller.OnPointerReleased({900, 700});

        const auto bounds = editor.document.GetShapes().back().GetBounds();
        REQUIRE(bounds.Right() <= canvas.width);
        REQUIRE(bounds.Bottom() <= canvas.height);
    }

    SECTION("С обычным инструментом щелчок выделяет фигуру, а не создаёт новую") {
        editor.Add("прямоугольник", "rectangle", {100, 100, 200, 200});

        editor.controller.OnPointerPressed({150, 150});
        editor.controller.OnPointerReleased({150, 150});

        REQUIRE(editor.document.GetShapesCount() == 1);
        REQUIRE(editor.controller.GetSelectedId() == "прямоугольник");
    }
}

TEST_CASE("Курсор: подсветка маркера появляется только над холстом и только в покое") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {200, 200, 100, 100});
    editor.controller.OnPointerPressed({250, 250});
    editor.controller.OnPointerReleased({250, 250});

    const auto handle = editor::SelectionFrame::GetHandleCenters(
        editor.document.GetShapeBounds("прямоугольник")).front();

    SECTION("Над маркером маркер подсвечен") {
        editor.controller.OnPointerMoved(handle);
        REQUIRE(editor.controller.GetHoveredHandle() == editor::ResizeHandle::TopLeft);
    }

    SECTION("Внутри фигуры рядом с углом маркер не подсвечивается") {
        editor.controller.OnPointerMoved({210, 210});
        REQUIRE(editor.controller.GetHoveredHandle() == editor::ResizeHandle::None);
    }

    SECTION("Когда курсор ушёл с холста, подсветка сброшена") {
        editor.controller.OnPointerMoved(handle);
        editor.controller.OnPointerMoved({handle.x, -50});

        REQUIRE(editor.controller.GetHoveredHandle() == editor::ResizeHandle::None);
    }

    SECTION("Во время изменения размера подсветки нет") {
        editor.controller.OnPointerPressed(handle);
        editor.controller.OnPointerMoved(handle);

        REQUIRE(editor.controller.GetHoveredHandle() == editor::ResizeHandle::None);
        editor.controller.OnPointerReleased(handle);
    }
}

TEST_CASE("Курсор: маркер у самого края холста остаётся доступным") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {0, 0, 100, 100});

    editor.controller.OnPointerPressed({50, 50});
    editor.controller.OnPointerReleased({50, 50});

    const auto topLeft = editor::SelectionFrame::GetHandleCenters(
        editor.document.GetShapeBounds("прямоугольник")).front();
    REQUIRE(topLeft == (gfx::Point{0, 0}));

    editor.controller.OnPointerMoved(topLeft);
    REQUIRE(editor.controller.GetHoveredHandle() == editor::ResizeHandle::TopLeft);
}

TEST_CASE("Зона захвата маркера крупнее самого квадратика") {
    Editor editor;
    editor.Add("прямоугольник", "rectangle", {200, 200, 100, 100});

    editor.controller.OnPointerPressed({250, 250});
    editor.controller.OnPointerReleased({250, 250});

    const auto center = editor::SelectionFrame::GetHandleCenter(
        editor.document.GetShapeBounds("прямоугольник"), editor::ResizeHandle::TopLeft);
    const auto grab = editor::SelectionFrame::GetHandleGrabBounds(center);
    const double half = editor::SelectionFrame::handleSize / 2;

    SECTION("Квадратик целиком попадает в зону захвата") {
        REQUIRE(grab.Contains({center.x - half, center.y - half}));
        REQUIRE(grab.Contains({center.x + half, center.y + half}));
    }

    SECTION("Зона захвата симметрична относительно квадратика") {
        const double reach = editor::SelectionFrame::handleGrabSize / 2;

        REQUIRE(grab.x == center.x - reach);
        REQUIRE(grab.Right() == center.x + reach);
        REQUIRE(grab.y == center.y - reach);
        REQUIRE(grab.Bottom() == center.y + reach);
    }

    SECTION("Клик выше квадратика тоже тянет маркер") {
        const gfx::Point above{center.x, center.y - 7};

        editor.controller.OnPointerMoved(above);
        REQUIRE(editor.controller.GetHoveredHandle() == editor::ResizeHandle::TopLeft);

        editor.controller.OnPointerPressed(above);
        editor.controller.OnPointerMoved({above.x, above.y - 30});
        editor.controller.OnPointerReleased({above.x, above.y - 30});

        REQUIRE(editor.BoundsOf("прямоугольник").y == 170);
    }

    SECTION("Клик ниже квадратика тоже тянет маркер") {
        const gfx::Point below{center.x, center.y + 7};

        editor.controller.OnPointerMoved(below);
        REQUIRE(editor.controller.GetHoveredHandle() == editor::ResizeHandle::TopLeft);
    }
}

TEST_CASE("Работа с файлами: Ctrl+S, Ctrl+O и Ctrl+N через команды редактора") {
    Editor editor;
    editor.Add("первая", "rectangle", {10, 20, 100, 50});
    editor.controller.OnPointerPressed({50, 40});
    editor.controller.OnPointerReleased({50, 40});

    const auto path = std::filesystem::temp_directory_path() / "editor_document_test.shapes";
    std::filesystem::remove(path);

    SECTION("Сохранение без выбранного файла просит показать диалог") {
        int prompts = 0;
        editor.controller.OnSaveFileRequested().connect([&prompts] { ++prompts; });

        editor.controller.OnCommand(editor::UiCommand::SaveDocument);

        REQUIRE(prompts == 1);
    }

    SECTION("Сохранение и загрузка восстанавливают документ без выделения") {
        REQUIRE(editor.controller.SaveDocument(path));

        editor.controller.NewDocument();
        REQUIRE(editor.document.GetShapesCount() == 0);

        REQUIRE(editor.controller.LoadDocument(path));
        REQUIRE(editor.document.GetShapesCount() == 1);
        REQUIRE(editor.document.GetShapes().front().GetId() == "первая");
        REQUIRE(editor.document.GetShapes().front().GetBounds() == (gfx::Rect{10, 20, 100, 50}));
        REQUIRE_FALSE(editor.controller.GetSelectedId().has_value());
    }

    SECTION("Ошибка загрузки не разрушает текущий документ") {
        REQUIRE(editor.controller.SaveDocument(path));
        editor.controller.NewDocument();
        editor.Add("текущая", "ellipse", {1, 2, 30, 40});

        const auto broken = std::filesystem::temp_directory_path() / "editor_broken_test.shapes";
        {
            std::ofstream file(broken);
            file << "shapes 1\nshape circle непонятно 0 0 10 10 #FFFFFF\n";
        }

        REQUIRE_FALSE(editor.controller.LoadDocument(broken));
        REQUIRE_FALSE(editor.controller.GetLastError().empty());
        REQUIRE(editor.document.GetShapesCount() == 1);
        REQUIRE(editor.document.GetShapes().front().GetId() == "текущая");

        std::filesystem::remove(broken);
    }

    SECTION("Отсутствующий файл даёт понятное сообщение") {
        REQUIRE_FALSE(editor.controller.LoadDocument("/tmp/файла-точно-нет.shapes"));
        REQUIRE_FALSE(editor.controller.GetLastError().empty());
    }

    SECTION("Ctrl+N очищает документ и путь к файлу") {
        REQUIRE(editor.controller.SaveDocument(path));
        REQUIRE_FALSE(editor.controller.GetFilePath().empty());

        editor.controller.OnCommand(editor::UiCommand::NewDocument);

        REQUIRE(editor.document.GetShapesCount() == 0);
        REQUIRE(editor.controller.GetFilePath().empty());
    }

    std::filesystem::remove(path);
}