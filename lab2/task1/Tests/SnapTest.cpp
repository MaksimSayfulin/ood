#include <sstream>
#include <string>

#include "../Catch/Catch.hpp"
#include "../Editor/EditorController.h"
#include "../Editor/SelectionFrame.h"
#include "../Editor/SnapEngine.h"
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

TEST_CASE("Привязка: фигура притягивается к краю соседней") {
    Editor editor;
    editor.Add("опора", "rectangle", {100, 100, 100, 100});
    editor.Add("двигаемая", "ellipse", {400, 300, 80, 80});

    editor.controller.OnPointerPressed({440, 340});
    editor.controller.OnPointerReleased({440, 340});

    editor.controller.OnPointerPressed({440, 340});
    editor.controller.OnPointerMoved({243, 340});

    REQUIRE(editor.BoundsOf("двигаемая").x == 200);
    REQUIRE(editor.controller.GetActiveGuides().size() == 1);
    REQUIRE(editor.controller.GetActiveGuides().front().axis == editor::GuideLine::Axis::Vertical);
    REQUIRE(editor.controller.GetActiveGuides().front().position == 200);
}

TEST_CASE("Привязка: фигура притягивается к центру соседней") {
    Editor editor;
    editor.Add("опора", "rectangle", {100, 100, 100, 100});
    editor.Add("двигаемая", "ellipse", {400, 300, 80, 80});

    editor.controller.OnPointerPressed({440, 340});
    editor.controller.OnPointerReleased({440, 340});

    editor.controller.OnPointerPressed({440, 340});
    editor.controller.OnPointerMoved({153, 340});

    REQUIRE(editor.BoundsOf("двигаемая").x + 40 == 150);
}

TEST_CASE("Привязка: при слишком большом расстоянии фигура не притягивается") {
    Editor editor;
    editor.Add("опора", "rectangle", {100, 100, 100, 100});
    editor.Add("двигаемая", "ellipse", {400, 300, 80, 80});

    editor.controller.OnPointerPressed({440, 340});
    editor.controller.OnPointerReleased({440, 340});

    editor.controller.OnPointerPressed({440, 340});
    editor.controller.OnPointerMoved({390, 340});

    REQUIRE(editor.BoundsOf("двигаемая").x == 350);
    REQUIRE(editor.controller.GetActiveGuides().empty());
}

TEST_CASE("Привязка: привязку можно временно отключить") {
    Editor editor;
    editor.Add("опора", "rectangle", {100, 100, 100, 100});
    editor.Add("двигаемая", "ellipse", {400, 300, 80, 80});

    editor.controller.OnPointerPressed({440, 340});
    editor.controller.OnPointerReleased({440, 340});

    editor.controller.OnPointerPressed({440, 340});
    editor.controller.OnPointerMoved({405, 340}, true);

    REQUIRE(editor.BoundsOf("двигаемая").x == 365);
    REQUIRE(editor.controller.GetActiveGuides().empty());
}

TEST_CASE("Привязка: при изменении размера тянется только тот край, который тянут") {
    Editor editor;
    editor.Add("опора", "rectangle", {400, 100, 100, 100});
    editor.Add("двигаемая", "rectangle", {100, 300, 100, 100});

    editor.controller.OnPointerPressed({150, 350});
    editor.controller.OnPointerReleased({150, 350});

    const auto handle = editor::SelectionFrame::GetHandleCenter(
        editor.BoundsOf("двигаемая"), editor::ResizeHandle::Right);

    editor.controller.OnPointerPressed(handle);
    editor.controller.OnPointerMoved({395, handle.y});

    REQUIRE(editor.BoundsOf("двигаемая").Right() == 400);
}

TEST_CASE("Привязка: движется только выбранная фигура, соседние не притягиваются") {
    Editor editor;
    editor.Add("опора", "rectangle", {100, 100, 100, 100});
    editor.Add("двигаемая", "ellipse", {400, 300, 80, 80});

    editor.controller.OnPointerPressed({440, 340});
    editor.controller.OnPointerReleased({440, 340});

    editor.controller.OnPointerPressed({440, 340});
    editor.controller.OnPointerMoved({243, 340});
    editor.controller.OnPointerReleased({243, 340});

    REQUIRE(editor.BoundsOf("опора").x == 100);
    REQUIRE(editor.BoundsOf("двигаемая").x == 200);
}

TEST_CASE("Привязка: после завершения жеста направляющие исчезают") {
    Editor editor;
    editor.Add("опора", "rectangle", {100, 100, 100, 100});
    editor.Add("двигаемая", "ellipse", {400, 300, 80, 80});

    editor.controller.OnPointerPressed({440, 340});
    editor.controller.OnPointerReleased({440, 340});

    editor.controller.OnPointerPressed({440, 340});
    editor.controller.OnPointerMoved({243, 340});
    REQUIRE_FALSE(editor.controller.GetActiveGuides().empty());

    editor.controller.OnPointerReleased({243, 340});
    REQUIRE(editor.controller.GetActiveGuides().empty());
}

TEST_CASE("Привязка: поиск опорных линий умеет выбирать нужные точки фигуры") {
    const std::vector<gfx::Rect> others{{100, 100, 100, 100}};

    SECTION("Левый край притягивается к краю соседа") {
        const auto snap = editor::SnapEngine::Find({205, 300, 50, 50}, others, editor::SnapEngine::Left);

        REQUIRE(snap.dx == -5);
        REQUIRE(snap.guides.size() == 1);
        REQUIRE(snap.guides.front().position == 200);
    }

    SECTION("Правый край не притягивается, если тянут левый") {
        const auto snap = editor::SnapEngine::Find({195, 300, 50, 50}, others, editor::SnapEngine::Left);

        REQUIRE(snap.dx == 5);
        REQUIRE(snap.guides.front().position == 200);
    }

    SECTION("Без указания точек привязки не происходит") {
        const auto snap = editor::SnapEngine::Find({200, 300, 50, 50}, others, 0);

        REQUIRE(snap.dx == 0);
        REQUIRE(snap.guides.empty());
    }
}
