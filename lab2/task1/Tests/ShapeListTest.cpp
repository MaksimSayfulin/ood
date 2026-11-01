#include <sstream>
#include <string>

#include "../Catch/Catch.hpp"
#include "../Editor/EditorController.h"
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

        [[nodiscard]] std::string TopShapeId() const {
            return document.GetShapes().back().GetId();
        }

        [[nodiscard]] std::string BottomShapeId() const {
            return document.GetShapes().front().GetId();
        }
    };
}

TEST_CASE("Список слоёв: верхняя строка соответствует верхнему слою документа") {
    Editor editor;
    editor.Add("нижняя", "rectangle", {100, 100, 100, 100});
    editor.Add("средняя", "ellipse", {200, 200, 100, 100});
    editor.Add("верхняя", "triangle", {300, 300, 100, 100});

    SECTION("Первая строка — верхний слой") {
        editor.controller.OnListRowPressed(0);
        REQUIRE(editor.controller.GetSelectedId() == "верхняя");
    }

    SECTION("Последняя строка — нижний слой") {
        editor.controller.OnListRowPressed(2);
        REQUIRE(editor.controller.GetSelectedId() == "нижняя");
    }

    SECTION("Строка за пределами списка ничего не выделяет") {
        editor.controller.OnListRowPressed(std::nullopt);
        REQUIRE_FALSE(editor.controller.GetSelectedId().has_value());
    }
}

TEST_CASE("Список слоёв: выбор строки синхронизирован с выделением на холсте") {
    Editor editor;
    editor.Add("первая", "rectangle", {100, 100, 100, 100});
    editor.Add("вторая", "ellipse", {300, 300, 100, 100});

    editor.controller.OnListRowPressed(0);
    REQUIRE(editor.controller.GetSelectedId() == "вторая");

    editor.controller.OnPointerPressed({150, 150});
    editor.controller.OnPointerReleased({150, 150});

    REQUIRE(editor.controller.GetSelectedId() == "первая");
    REQUIRE(editor.controller.GetSelectedIds().size() == 1);
}

TEST_CASE("Список слоёв: перетаскивание строки меняет порядок слоёв") {
    Editor editor;
    editor.Add("нижняя", "rectangle", {100, 100, 100, 100});
    editor.Add("средняя", "ellipse", {200, 200, 100, 100});
    editor.Add("верхняя", "triangle", {300, 300, 100, 100});

    REQUIRE(editor.TopShapeId() == "верхняя");

    SECTION("Фигура, перетащенная наверх, становится верхним слоем") {
        editor.controller.OnListRowPressed(2);
        editor.controller.OnListRowMoved(0);
        editor.controller.OnListRowReleased();

        REQUIRE(editor.TopShapeId() == "нижняя");
        REQUIRE(editor.BottomShapeId() == "средняя");
    }

    SECTION("Фигура, перетащенная вниз, становится нижним слоем") {
        editor.controller.OnListRowPressed(0);
        editor.controller.OnListRowMoved(2);
        editor.controller.OnListRowReleased();

        REQUIRE(editor.TopShapeId() == "средняя");
        REQUIRE(editor.BottomShapeId() == "верхняя");
    }
}

TEST_CASE("Список слоёв: изменение порядка не ломает связи фигур с моделью") {
    Editor editor;
    editor.Add("первая", "rectangle", {100, 100, 100, 100});
    editor.Add("вторая", "ellipse", {200, 200, 100, 100});

    editor.controller.OnListRowPressed(0);
    editor.controller.OnListRowMoved(1);
    editor.controller.OnListRowReleased();

    REQUIRE(editor.document.GetShapesCount() == 2);
    REQUIRE(editor.TopShapeId() == "первая");
    REQUIRE(editor.BottomShapeId() == "вторая");
}

TEST_CASE("Список слоёв: прокрутка ограничена размером списка") {
    Editor editor;
    for (int i = 0; i < 40; ++i) {
        editor.Add("фигура" + std::to_string(i), "rectangle", {100.0 * (i % 7), 100.0 * (i % 5), 50, 50});
    }

    REQUIRE(editor.controller.GetListScroll() == 0);

    editor.controller.OnListScrolled(1);
    REQUIRE(editor.controller.GetListScroll() > 0);

    editor.controller.OnListScrolled(-100);
    REQUIRE(editor.controller.GetListScroll() == 0);
}
