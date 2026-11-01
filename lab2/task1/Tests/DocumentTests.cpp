#include <sstream>

#include "../Catch/Catch.hpp"
#include "../Graphics/Canvas/CanvasSVG/CanvasSvg.h"
#include "../Picture/model/DocumentSerializer.h"
#include "../Picture/model/Picture.h"
#include "../Shape/model/ShapeFactory.h"

namespace {
    shapes::IShape MakeShape(const std::string& id, const std::string& type, const gfx::Rect& bounds) {
        return {id, gfx::Color("#4363D8"), shapes::ShapeFactory::CreateGeometry(type, bounds)};
    }

    std::string SaveToText(const std::vector<shapes::IShape>& shapes) {
        std::ostringstream output;
        shapes::DocumentSerializer::Save(shapes, output);
        return output.str();
    }

    std::string LoadErrorMessage(const std::string& text) {
        try {
            std::istringstream input(text);
            shapes::DocumentSerializer::Load(input);
        } catch (const shapes::DocumentError& error) {
            return error.what();
        }
        return {};
    }
}

TEST_CASE("Документ: сохранение и загрузка сохраняют фигуры, типы и порядок слоёв") {
    const std::vector<shapes::IShape> original = {
        MakeShape("a", "rectangle", {10, 20, 100, 50}),
        MakeShape("b", "ellipse", {200, 30, 80, 80}),
        MakeShape("c", "triangle", {300, 40, 60, 90})
    };

    SECTION("Круговой обмен данными не меняет документ") {
        std::istringstream input(SaveToText(original));
        const auto loaded = shapes::DocumentSerializer::Load(input);

        REQUIRE(loaded.size() == 3);

        for (std::size_t i = 0; i < original.size(); ++i) {
            REQUIRE(loaded[i].GetId() == original[i].GetId());
            REQUIRE(loaded[i].GetGeometry()->GetShapeName() == original[i].GetGeometry()->GetShapeName());
            REQUIRE(loaded[i].GetBounds() == original[i].GetBounds());
            REQUIRE(loaded[i].GetColor() == original[i].GetColor());
        }
    }

    SECTION("Порядок строк в файле совпадает с порядком слоёв") {
        const auto text = SaveToText(original);

        REQUIRE(text.find("shape rectangle a") < text.find("shape ellipse b"));
        REQUIRE(text.find("shape ellipse b") < text.find("shape triangle c"));
    }

    SECTION("Пустой документ сохраняется и загружается без фигур") {
        std::istringstream input(SaveToText({}));
        REQUIRE(shapes::DocumentSerializer::Load(input).empty());
    }

    SECTION("Комментарии и пустые строки не мешают разбору") {
        std::istringstream input("# комментарий\n\nshapes 1\n\nshape rectangle a 0 0 10 10 #FFFFFF\n");
        REQUIRE(shapes::DocumentSerializer::Load(input).size() == 1);
    }
}

TEST_CASE("Документ: некорректный файл приводит к понятной ошибке") {
    SECTION("Файл не является документом") {
        REQUIRE_FALSE(LoadErrorMessage("hello world\n").empty());
    }

    SECTION("Файл пуст") {
        REQUIRE_FALSE(LoadErrorMessage("").empty());
    }

    SECTION("Неподдерживаемая версия") {
        REQUIRE_FALSE(LoadErrorMessage("shapes 42\n").empty());
    }

    SECTION("Неизвестный тип фигуры") {
        REQUIRE(LoadErrorMessage("shapes 1\nshape circle a 0 0 10 10 #FFFFFF\n").contains("circle"));
    }

    SECTION("Не хватает обязательных данных") {
        REQUIRE_FALSE(LoadErrorMessage("shapes 1\nshape rectangle a 0 0 10\n").empty());
    }

    SECTION("Параметр не является числом") {
        REQUIRE_FALSE(LoadErrorMessage("shapes 1\nshape rectangle a x 0 10 10 #FFFFFF\n").empty());
    }

    SECTION("Некорректный цвет") {
        REQUIRE_FALSE(LoadErrorMessage("shapes 1\nshape rectangle a 0 0 10 10 красный\n").empty());
    }

    SECTION("Отрицательные размеры") {
        REQUIRE_FALSE(LoadErrorMessage("shapes 1\nshape rectangle a 0 0 -10 10 #FFFFFF\n").empty());
    }

    SECTION("Повторяющийся идентификатор") {
        REQUIRE_FALSE(LoadErrorMessage("shapes 1\n"
                                       "shape rectangle a 0 0 10 10 #FFFFFF\n"
                                       "shape ellipse a 5 5 10 10 #FFFFFF\n")
                          .empty());
    }

    SECTION("Неизвестная команда") {
        REQUIRE_FALSE(LoadErrorMessage("shapes 1\nremove everything\n").empty());
    }
}

TEST_CASE("Документ: неудачная загрузка не разрушает текущее состояние") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);
    picture.AddShape(MakeShape("исходная", "rectangle", {10, 10, 50, 50}));

    std::istringstream input("shapes 1\nshape circle bad 0 0 10 10 #FFFFFF\n");
    REQUIRE_THROWS(shapes::DocumentSerializer::Load(input));

    REQUIRE(picture.GetShapesCount() == 1);
    REQUIRE(picture.GetShape("исходная").GetBounds() == (gfx::Rect{10, 10, 50, 50}));
}

TEST_CASE("Документ: ReplaceWith заменяет содержимое целиком") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    picture.AddShape(MakeShape("старая", "rectangle", {0, 0, 10, 10}));
    picture.ReplaceWith({MakeShape("новая", "ellipse", {5, 5, 20, 20})});

    REQUIRE(picture.GetShapesCount() == 1);
    REQUIRE(picture.GetShape("новая").GetGeometry()->GetShapeName() == "ellipse");
}

TEST_CASE("Документ: порядок слоёв меняется по требованию редактора") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    picture.AddShape(MakeShape("нижняя", "rectangle", {0, 0, 10, 10}));
    picture.AddShape(MakeShape("средняя", "ellipse", {0, 0, 10, 10}));
    picture.AddShape(MakeShape("верхняя", "triangle", {0, 0, 10, 10}));

    SECTION("Фигура переносится на указанную позицию") {
        picture.MoveShapeToIndex("нижняя", 2);

        REQUIRE(picture.GetShapes().back().GetId() == "нижняя");
        REQUIRE(picture.GetShapeIndex("нижняя") == 2);
    }

    SECTION("Порядок слоёв сохраняется в файле") {
        picture.MoveShapeToIndex("верхняя", 0);

        const auto text = SaveToText(picture.GetShapes());
        REQUIRE(text.find("shape triangle верхняя") < text.find("shape rectangle нижняя"));
    }

    SECTION("Изменение порядка сообщает наблюдателям") {
        int notifications = 0;
        picture.OnOrderChanged().connect([&notifications] { ++notifications; });

        picture.MoveShapeToIndex("средняя", 0);
        REQUIRE(notifications == 1);

        picture.MoveShapeToIndex("средняя", 0);
        REQUIRE(notifications == 1);
    }

    SECTION("После перестановки фигуры остаются живыми") {
        picture.MoveShapeToIndex("средняя", 2);

        int moved = 0;
        picture.OnShapeMoved().connect([&moved](const shapes::IShape&, double, double) { ++moved; });

        picture.MoveShape("средняя", 5, 5);
        REQUIRE(moved == 1);
        REQUIRE(picture.GetShape("средняя").GetBounds() == (gfx::Rect{5, 5, 10, 10}));
    }
}